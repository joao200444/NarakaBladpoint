// Frida script para executar PARRY diretamente
// Uso: python -m frida_tools.repl -p <PID> -l parry.js

console.log("[*] Parry Execution Script iniciado");

var gameAssembly = Process.findModuleByName("GameAssembly_Super.dll");

if (gameAssembly) {
    console.log("[+] GameAssembly_Super.dll: " + gameAssembly.base);
    
    // RVAs importantes para parry (ATUALIZADOS DO Assembly-CSharp.h)
    var RVA_GET_MAIN_PLAYER = 0x4de74c0;  // CharacterManager.GetMainPlayerActorModel
    var RVA_ON_PARRY = 0x56dfc60;  // CharacterBehaviorHandler.OnParry
    var RVA_ENTER_BLOCK_PARRY = 0x63ba600;  // BlockParryAgent.EnterBlockParry
    var RVA_GET_BLOCK_PARRY_AGENT = 0x822aaf0; // GetBlockParryAgentPropertyData
    var RVA_IS_LOADING = 0x83ac280;  // IsLoading (detecta se está carregando)
    
    var addr_get_main_player = gameAssembly.base.add(RVA_GET_MAIN_PLAYER);
    var addr_on_parry = gameAssembly.base.add(RVA_ON_PARRY);
    var addr_enter_block_parry = gameAssembly.base.add(RVA_ENTER_BLOCK_PARRY);
    var addr_get_block_parry_agent = gameAssembly.base.add(RVA_GET_BLOCK_PARRY_AGENT);
    
    console.log("[+] GetMainPlayer: " + addr_get_main_player);
    console.log("[+] OnParry: " + addr_on_parry);
    console.log("[+] EnterBlockParry: " + addr_enter_block_parry);
    
    var g_characterManagerInstance = null;
    var g_localPlayer = null;
    var g_blockParryAgent = null;
    var g_localPlayerCaptured = false;
    var g_behaviorHandler = null;
    var g_isLoading = false;  // Flag para detectar carregamento
    
    // Função auxiliar para SendInput (método do AutoHotkey)
    var user32Module = Process.getModuleByName("user32.dll");
    var sendInputAddr = user32Module.getExportByName("SendInput");
    var sendInput = new NativeFunction(sendInputAddr, 'uint32', ['uint32', 'pointer', 'int']);
    
    console.log("[+] SendInput: " + sendInputAddr);
    
    // Estrutura INPUT para SendInput
    function createKeyInput(scanCode, isKeyUp) {
        var input = Memory.alloc(40);  // sizeof(INPUT)
        
        // INPUT.type = INPUT_KEYBOARD (1)
        input.writeU32(1);
        
        // INPUT.ki.wVk = 0 (usar scan code)
        input.add(8).writeU16(0);
        
        // INPUT.ki.wScan = scanCode
        input.add(10).writeU16(scanCode);
        
        // INPUT.ki.dwFlags = KEYEVENTF_SCANCODE | KEYEVENTF_KEYUP (se for keyup)
        var flags = 0x0008;  // KEYEVENTF_SCANCODE
        if (isKeyUp) {
            flags |= 0x0002;  // KEYEVENTF_KEYUP
        }
        input.add(12).writeU32(flags);
        
        // INPUT.ki.time = 0
        input.add(16).writeU32(0);
        
        // INPUT.ki.dwExtraInfo = 0
        input.add(20).writePointer(ptr(0));
        
        return input;
    }
    
    // Hook IsLoading para detectar carregamento
    Interceptor.attach(gameAssembly.base.add(RVA_IS_LOADING), {
        onLeave: function(retval) {
            var wasLoading = g_isLoading;
            g_isLoading = retval.toInt32() !== 0;
            
            if (wasLoading !== g_isLoading) {
                if (g_isLoading) {
                    console.log("[LOADING] Carregando... (parry desativado)");
                } else {
                    console.log("[LOADING] Carregamento completo! (parry ativado)");
                }
            }
        }
    });
    
    // Hook GetKeyAsync para detectar tecla L
    var user32Module = Process.getModuleByName("user32.dll");
    var getAsyncKeyStateAddr = user32Module.getExportByName("GetAsyncKeyState");
    var getAsyncKeyState = new NativeFunction(getAsyncKeyStateAddr, 'int16', ['int']);
    
    console.log("[+] GetAsyncKeyState: " + getAsyncKeyStateAddr);
    
    // Scan code da tecla L = 0x26, VK_L = 0x4C
    var VK_L = 0x4C;
    
    // Thread para monitorar tecla L
    setInterval(function() {
        try {
            // Só executa parry se não estiver carregando
            if (!g_isLoading) {
                var keyState = getAsyncKeyState(VK_L);
                
                // Bit 15 setado = key is currently down
                // Usamos & 0x8000 para verificar se está pressionado
                if ((keyState & 0x8000) !== 0) {
                    // Tecla L pressionada - executa parry
                    executeParry();
                    
                    // Pequeno delay para evitar spam excessivo
                    Thread.sleep(100);
                }
            }
        } catch(e) {
            // Ignora erros
        }
    }, 50);  // Checa a cada 50ms
    Interceptor.attach(addr_get_main_player, {
        onEnter: function(args) {
            var thisPtr = this.context.rcx;
            if (!thisPtr.isNull() && !g_characterManagerInstance) {
                g_characterManagerInstance = thisPtr;
                console.log("[+] CharacterManager capturado: " + thisPtr);
            }
        },
        onLeave: function(retval) {
            if (!retval.isNull()) {
                g_localPlayer = retval;
                
                if (!g_localPlayerCaptured) {
                    g_localPlayerCaptured = true;
                    console.log("[+] Jogador LOCAL capturado: " + g_localPlayer);
                    
                    // Tentar obter BlockParryAgent
                    tryGetBlockParryAgent();
                }
            }
        }
    });
    
    // Função para obter CharacterBehaviorHandler
    function tryGetBehaviorHandler() {
        try {
            if (!g_localPlayer || g_localPlayer.isNull()) {
                console.log("[-] Local player não capturado");
                return null;
            }
            
            // CharacterBehaviorHandler geralmente está em offset do actor
            // Vamos procurar testando OnParry em cada offset
            var offsets = [0x50, 0x58, 0x60, 0x68, 0x70, 0x78, 0x80, 0x88, 0x90, 0x98, 0xa0, 0xa8, 0xb0, 0xb8, 0xc0];
            
            console.log("[*] Testando offsets para BehaviorHandler...");
            
            for (var i = 0; i < offsets.length; i++) {
                try {
                    var offset = offsets[i];
                    var possibleHandler = g_localPlayer.add(offset).readPointer();
                    
                    if (!possibleHandler.isNull() && possibleHandler.compare(ptr(0x1000)) > 0) {
                        console.log("[?] Testando offset 0x" + offset.toString(16) + ": " + possibleHandler);
                        
                        // Tentar chamar OnParry para testar
                        try {
                            var onParry = new NativeFunction(addr_on_parry, 'void', ['pointer', 'int']);
                            onParry(possibleHandler, 1);
                            
                            console.log("[+] ✓ BehaviorHandler VÁLIDO encontrado em offset 0x" + offset.toString(16) + ": " + possibleHandler);
                            console.log("[+] ✓ PARRY EXECUTADO COM SUCESSO!");
                            g_behaviorHandler = possibleHandler;
                            return possibleHandler;
                        } catch(e) {
                            // Não é o handler correto, continuar
                            console.log("[-] Offset 0x" + offset.toString(16) + " falhou: " + e.message);
                        }
                    }
                } catch(e) {}
            }
            
            console.log("[-] BehaviorHandler não encontrado");
            return null;
            
        } catch(e) {
            console.log("[-] Erro ao obter BehaviorHandler: " + e);
            return null;
        }
    }
    
    // Função para obter BlockParryAgent
    function tryGetBlockParryAgent() {
        try {
            if (!g_localPlayer || g_localPlayer.isNull()) {
                console.log("[-] Local player não capturado");
                return null;
            }
            
            // Offset correto: 0x1e0
            var possibleAgent = g_localPlayer.add(0x1e0).readPointer();
            
            if (!possibleAgent.isNull() && possibleAgent.compare(ptr(0x1000)) > 0) {
                console.log("[+] BlockParryAgent encontrado: " + possibleAgent);
                g_blockParryAgent = possibleAgent;
                return possibleAgent;
            }
            
            console.log("[-] BlockParryAgent não encontrado");
            return null;
            
        } catch(e) {
            console.log("[-] Erro ao obter BlockParryAgent: " + e);
            return null;
        }
    }
    
    // Função para executar parry com spam de L (método AutoHotkey)
    function executeParry() {
        try {
            console.log("\n[*] ===== EXECUTANDO PARRY (SPAM L - MÉTODO AUTOHOTKEY) =====");
            
            // Scan code da tecla L = 0x26
            var scanCodeL = 0x26;
            
            // Spam de L: 30 cliques em 1 segundo
            var clicks = 30;
            var delayMs = Math.floor(1000 / clicks);  // ~33ms entre cada clique
            
            console.log("[+] Enviando " + clicks + " cliques de L (scan code) em 1 segundo...");
            
            for (var i = 0; i < clicks; i++) {
                // Key down
                var inputDown = createKeyInput(scanCodeL, false);
                sendInput(1, inputDown, 40);
                
                // Pequeno delay
                Thread.sleep(delayMs / 2000);
                
                // Key up
                var inputUp = createKeyInput(scanCodeL, true);
                sendInput(1, inputUp, 40);
                
                // Delay até próximo clique
                Thread.sleep(delayMs / 2000);
            }
            
            console.log("[+] ✓ " + clicks + " CLIQUES DE L ENVIADOS (MÉTODO AUTOHOTKEY)!");
            console.log("[*] ===== FIM =====\n");
            return true;
            
        } catch(e) {
            console.log("[-] Erro ao executar parry: " + e);
            console.log("[-] Stack: " + e.stack);
            return false;
        }
    }
    
    // Exportar função
    this.executeParry = executeParry;
    this.tryGetBlockParryAgent = tryGetBlockParryAgent;
    this.tryGetBehaviorHandler = tryGetBehaviorHandler;
    
    console.log("\n[*] Funções disponíveis:");
    console.log("  - executeParry()           : Executa parry manualmente");
    console.log("  - tryGetBlockParryAgent()  : Tenta encontrar BlockParryAgent");
    console.log("\n[+] Script carregado!");
    console.log("\n[!] INSTRUÇÕES:");
    console.log("  1. Entre em uma partida");
    console.log("  2. Pressione a tecla L para executar parry automaticamente");
    console.log("  3. O parry é desativado durante carregamentos");
    console.log("  4. Ou execute manualmente: executeParry()");
    
} else {
    console.log("[-] GameAssembly_Super.dll não encontrado!");
}
