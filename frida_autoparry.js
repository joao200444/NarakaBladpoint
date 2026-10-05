// Frida script para monitorar ATAQUES CARREGADOS (Charged Attacks)
// Foco: Detectar estágios 1, 2, 3 quando VOCÊ ou INIMIGOS carregam ataque
// Uso: python -m frida_tools.repl -p <PID> -l frida_autoparry.js

console.log("[*] Charged Attack Monitor Script iniciado");
console.log("[!] OBJETIVO: Detectar estágios de ataques carregados via SyncComboChargeStagePacket");

var gameAssembly = Process.findModuleByName("GameAssembly_Super.dll");

if (gameAssembly) {
    console.log("[+] GameAssembly_Super.dll: " + gameAssembly.base);
    
    // RVAs importantes (ATUALIZADOS DO Assembly-CSharp.h)
    var RVA_GET_MAIN_PLAYER = 0x4de74c0;              // CharacterManager.GetMainPlayerActorModel
    var RVA_GET_ALIVE_CHAR_LIST = 0x540ff20;           // CharacterManager.GetAliveCharacterFidList
    var RVA_GET_POSITION = 0x6db0690;                 // Transform.get_Position
    var RVA_CHECK_COMBO_CHARGE_STAGE = 0xd6e26c0;     // CheckComboChargeStage
    var RVA_REMOTE_CHARGE_STAGE = 0xd6ef310;           // RemoteChargeStage
    var RVA_COMBO_RELEASE = 0xd6bee40;                 // ComboRelease
    var RVA_EXEC_COMBO = 0xd6cb5a0;                   // ExecCombo
    
    // Endereços
    var addr_get_main_player = gameAssembly.base.add(RVA_GET_MAIN_PLAYER);
    var addr_get_alive_list = gameAssembly.base.add(RVA_GET_ALIVE_CHAR_LIST);
    var addr_get_position = gameAssembly.base.add(RVA_GET_POSITION);
    var addr_check_charge = gameAssembly.base.add(RVA_CHECK_COMBO_CHARGE_STAGE);
    var addr_remote_charge = gameAssembly.base.add(RVA_REMOTE_CHARGE_STAGE);
    var addr_combo_release = gameAssembly.base.add(RVA_COMBO_RELEASE);
    var addr_exec_combo = gameAssembly.base.add(RVA_EXEC_COMBO);
    
    console.log("[+] CheckComboChargeStage: " + addr_check_charge);
    console.log("[+] RemoteChargeStage: " + addr_remote_charge);
    console.log("[+] ComboRelease: " + addr_combo_release);
    console.log("[+] ExecCombo: " + addr_exec_combo);
    
    var g_characterManagerInstance = null;
    var g_localPlayer = null;
    
    // Função auxiliar para SendInput (simular tecla L)
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
    
    // Função para clicar L 1 vez (via SendInput) - assíncrono
    function autoParry() {
        try {
            console.log("[AUTO-PARRY] Clicando L 1 vez!");
            
            // Scan code da tecla L = 0x26
            var scanCodeL = 0x26;
            
            // Key down
            var inputDown = createKeyInput(scanCodeL, false);
            sendInput(1, inputDown, 40);
            
            // Usar setTimeout para key up (assíncrono)
            setTimeout(function() {
                var inputUp = createKeyInput(scanCodeL, true);
                sendInput(1, inputUp, 40);
                console.log("[AUTO-PARRY] ✓ 1 clique de L enviado!");
            }, 10);
            
        } catch(e) {
            console.log("[-] Erro no auto-parry: " + e);
        }
    }
    
    // Hook para capturar CharacterManager e jogador local
    Interceptor.attach(addr_get_main_player, {
        onEnter: function(args) {
            var thisPtr = this.context.rcx;
            if (!thisPtr.isNull() && !g_characterManagerInstance) {
                g_characterManagerInstance = thisPtr;
                console.log("[+] CharacterManager capturado: " + thisPtr);
            }
        },
        onLeave: function(retval) {
            if (!retval.isNull() && !g_localPlayer) {
                g_localPlayer = retval;
                console.log("[+] Jogador LOCAL capturado: " + g_localPlayer);
            }
        }
    });
    
    // ===== HOOK 1: CheckComboChargeStage - Quando VOCÊ carrega =====
    // Este hook é chamado mas não tem o estágio como parâmetro
    // Vamos apenas contar quantas vezes é chamado
    var checkChargeCount = 0;
    Interceptor.attach(addr_check_charge, {
        onEnter: function(args) {
            checkChargeCount++;
            var actor = args[0];
            
            if (actor.isNull()) return;
            
            console.log("\n[VOCÊ] CheckComboChargeStage chamado (contagem: " + checkChargeCount + ")");
            console.log("[VOCÊ] Actor: " + actor);
            console.log("[VOCÊ] Você está carregando o ataque...");
        }
    });
    
    // ===== HOOK 2: RemoteChargeStage - Quando INIMIGO carrega =====
    Interceptor.attach(addr_remote_charge, {
        onEnter: function(args) {
            // args[0] = ActorKit* (inimigo)
            // args[1] = int chargeStage
            var actor = args[0];
            var chargeStage = args[1].toInt32();
            
            if (actor.isNull()) return;
            
            console.log("\n[INIMIGO CARREGANDO] ===== RemoteChargeStage =====");
            console.log("[INIMIGO CARREGANDO] Actor: " + actor);
            console.log("[INIMIGO CARREGANDO] Estágio: " + chargeStage);
            
            // Mostrar significado do estágio
            var stageText = "";
            switch(chargeStage) {
                case 0: stageText = "Não carregado"; break;
                case 1: stageText = "Estágio 1 (início) ⚡"; break;
                case 2: stageText = "Estágio 2 (meio) ⚡⚡"; break;
                case 3: stageText = "Estágio 3 (MÁXIMO) ⚡⚡⚡ ← HORA DE PARRY!"; break;
                default: stageText = "Desconhecido"; break;
            }
            console.log("[INIMIGO CARREGANDO] Status: " + stageText);
            
            // Obter posição do inimigo
            var pos = getActorPosition(actor);
            if (pos) {
                console.log("[INIMIGO CARREGANDO] Posição: (" + 
                    pos.x.toFixed(1) + ", " + 
                    pos.y.toFixed(1) + ", " + 
                    pos.z.toFixed(1) + ")");
                    
                // Calcular distância do jogador local
                if (g_localPlayer) {
                    var localPos = getActorPosition(g_localPlayer);
                    if (localPos) {
                        var dx = pos.x - localPos.x;
                        var dy = pos.y - localPos.y;
                        var dz = pos.z - localPos.z;
                        var dist = Math.sqrt(dx*dx + dy*dy + dz*dz);
                        console.log("[INIMIGO CARREGANDO] Distância de você: " + dist.toFixed(1) + "m");
                        
                        if (dist < 5.0 && chargeStage === 3) {
                            console.log("[INIMIGO CARREGANDO] ⚠️⚠️⚠️ PERIGO! INIMIGO PERTO COM ATAQUE MÁXIMO! ⚠️⚠️⚠️");
                            console.log("[INIMIGO CARREGANDO] 🛡️ APERTE AZUL AGORA PARA PARRY! 🛡️");
                        }
                    }
                }
            }
            
            console.log("[INIMIGO CARREGANDO] ===== FIM =====\n");
        }
    });
    
    // ===== HOOK 3: ComboRelease - Quando ataque carregado é SOLTO =====
    Interceptor.attach(addr_combo_release, {
        onEnter: function(args) {
            // args[0] = ActorKit*
            // args[1] = ESkillInput (tipo de input)
            var actor = args[0];
            var skillInput = args[1].toInt32();
            
            if (actor.isNull()) return;
            
            // Verificar se é inimigo
            var isEnemy = !actor.equals(g_localPlayer);
            
            if (isEnemy) {
                console.log("\n[🚨 ATAQUE SOLTO!] ===== ComboRelease =====");
                console.log("[🚨 ATAQUE SOLTO!] INIMIGO SOLTOU ATAQUE CARREGADO!");
                console.log("[🚨 ATAQUE SOLTO!] Actor: " + actor);
                console.log("[🚨 ATAQUE SOLTO!] SkillInput: " + skillInput);
                
                // Obter posição e distância
                var pos = getActorPosition(actor);
                if (pos && g_localPlayer) {
                    var localPos = getActorPosition(g_localPlayer);
                    if (localPos) {
                        var dx = pos.x - localPos.x;
                        var dy = pos.y - localPos.y;
                        var dz = pos.z - localPos.z;
                        var dist = Math.sqrt(dx*dx + dy*dy + dz*dz);
                        
                        console.log("[🚨 ATAQUE SOLTO!] Distância: " + dist.toFixed(1) + "m");
                        
                        if (dist < 5.0) {
                            console.log("[🚨 ATAQUE SOLTO!] ⚠️⚠️⚠️ PERIGO! ATAQUE VINDO! ⚠️⚠️⚠️");
                            console.log("[🚨 ATAQUE SOLTO!] 🛡️🛡️🛡️ APERTE AZUL AGORA PARA PARRY! 🛡️🛡️🛡️");
                        }
                    }
                }
                
                console.log("[🚨 ATAQUE SOLTO!] ===== FIM =====\n");
            }
        }
    });
    
    // ===== HOOK 4: ExecCombo - Quando ataque é REALMENTE EXECUTADO (não cancelado) =====
    Interceptor.attach(addr_exec_combo, {
        onEnter: function(args) {
            // args[0] = ActorKit*
            // args[1] = ComboStateGroup
            // args[2] = ESkillInput
            // args[3] = inputIndex
            // args[4] = EComboType
            var actor = args[0];
            var skillInput = args[2].toInt32();
            var comboType = args[4].toInt32();
            
            if (actor.isNull()) return;
            
            // Verificar se é inimigo
            var isEnemy = !g_localPlayer || !actor.equals(g_localPlayer);
            
            if (isEnemy) {
                console.log("\n[⚔️ ATAQUE EXECUTADO!] ===== ExecCombo =====");
                console.log("[⚔️ ATAQUE EXECUTADO!] INIMIGO EXECUTOU ATAQUE!");
                console.log("[⚔️ ATAQUE EXECUTADO!] Actor: " + actor);
                console.log("[⚔️ ATAQUE EXECUTADO!] SkillInput: " + skillInput);
                console.log("[⚔️ ATAQUE EXECUTADO!] ComboType: " + comboType);
                console.log("[⚔️ ATAQUE EXECUTADO!] >>> AUTO-PARRY ATIVADO! <<<");
                autoParry();  // Clica L 3 vezes automaticamente
                console.log("[⚔️ ATAQUE EXECUTADO!] ===== FIM =====\n");
            }
        }
    });
    
    // Função para obter posição do actor
    function getActorPosition(actor) {
        try {
            for (var offset = 0x10; offset <= 0x100; offset += 0x8) {
                try {
                    var transformPtr = actor.add(offset).readPointer();
                    
                    if (!transformPtr.isNull() && transformPtr.compare(ptr(0x1000)) > 0) {
                        var posBuffer = Memory.alloc(12);
                        var getPosition = new NativeFunction(addr_get_position, 'void', ['pointer', 'pointer']);
                        getPosition(posBuffer, transformPtr);
                        
                        var x = posBuffer.readFloat();
                        var y = posBuffer.add(4).readFloat();
                        var z = posBuffer.add(8).readFloat();
                        
                        if (!isNaN(x) && !isNaN(y) && !isNaN(z) && 
                            Math.abs(x) < 100000 && Math.abs(y) < 100000 && Math.abs(z) < 100000) {
                            return { x: x, y: y, z: z, transform: transformPtr };
                        }
                    }
                } catch(e) {}
            }
            return null;
        } catch(e) {
            return null;
        }
    }
    
    // Função para obter actor por FID
    function getActorByFID(fid) {
        try {
            if (!g_characterManagerInstance) {
                return null;
            }
            
            var getAliveList = new NativeFunction(addr_get_alive_list, 'pointer', ['pointer']);
            var listObj = getAliveList(g_characterManagerInstance);
            
            if (listObj.isNull()) return null;
            
            var items = listObj.add(16).readPointer();
            var size = listObj.add(24).readInt();
            
            if (size === 0 || items.isNull()) return null;
            
            var itemsArray = items.add(32);
            
            // Primeiro actor = jogador local
            var localActor = itemsArray.readPointer();
            
            // Procurar actor com o FID
            for (var i = 0; i < size; i++) {
                var actor = itemsArray.add(i * 8).readPointer();
                
                if (actor.isNull()) continue;
                
                // Tentar ler FID do actor (offset pode variar)
                // Geralmente FID está em offset baixo
                try {
                    var actorFid = actor.add(0x18).readInt();
                    if (actorFid === fid) {
                        var isLocal = (i === 0);
                        var position = getActorPosition(actor);
                        
                        return {
                            actor: actor,
                            isLocal: isLocal,
                            position: position
                        };
                    }
                } catch(e) {}
            }
            
            return null;
            
        } catch(e) {
            return null;
        }
    }
    
    // Função para obter jogador local
    function getLocalPlayer() {
        try {
            if (!g_characterManagerInstance) {
                console.log("[-] CharacterManager não capturado");
                return null;
            }
            
            var getAliveList = new NativeFunction(addr_get_alive_list, 'pointer', ['pointer']);
            var listObj = getAliveList(g_characterManagerInstance);
            
            if (listObj.isNull()) return null;
            
            var items = listObj.add(16).readPointer();
            var size = listObj.add(24).readInt();
            
            if (size === 0 || items.isNull()) return null;
            
            var itemsArray = items.add(32);
            var localActor = itemsArray.readPointer();
            
            if (localActor.isNull()) return null;
            
            console.log("[+] Jogador local: " + localActor);
            return localActor;
            
        } catch(e) {
            return null;
        }
    }
    
    // Função para mostrar info
    function startMonitoring() {
        console.log("\n[*] ===== INICIANDO MONITORAMENTO =====");
        
        var player = getLocalPlayer();
        if (!player) {
            console.log("[-] Não foi possível obter jogador");
            console.log("[!] Entre em uma partida primeiro!");
            return;
        }
        
        console.log("[+] Jogador: " + player);
        
        // Obter posição
        var pos = getActorPosition(player);
        if (pos) {
            console.log("[+] Posição: (" + pos.x.toFixed(1) + ", " + pos.y.toFixed(1) + ", " + pos.z.toFixed(1) + ")");
        }
        
        console.log("\n[!] MONITORAMENTO ATIVO!");
        console.log("[!] Agora SEGURE o botão de ataque para carregar");
        console.log("[!] Os logs aparecerão automaticamente quando detectar:");
        console.log("    - VOCÊ carregando ataque (Estágio 1, 2, 3)");
        console.log("    - INIMIGOS carregando ataque");
        console.log("\n[!] Isso é útil para AUTO-PARRY!");
        console.log("[!] Quando ver 'INIMIGO Estágio 3', é hora de fazer PARRY!\n");
    }
    
    this.startMonitoring = startMonitoring;
    this.getLocalPlayer = getLocalPlayer;
    
    console.log("\n[*] Funções disponíveis:");
    console.log("  - startMonitoring()  : Inicia monitoramento");
    console.log("\n[+] Script carregado!");
    console.log("\n[!] INSTRUÇÕES:");
    console.log("  1. Entre em uma partida");
    console.log("  2. Execute: startMonitoring()");
    console.log("  3. SEGURE o botão de ataque (carregue)");
    console.log("  4. Veja os logs dos estágios 1, 2, 3!");
    console.log("  5. Também verá quando INIMIGOS carregam!");
    
} else {
    console.log("[-] GameAssembly_Super.dll não encontrado!");
}
