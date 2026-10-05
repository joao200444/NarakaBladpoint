// entity.js - Frida script para testar conexão com NarakaBladepoint.exe
// Uso: frida -p <PID> -l entity.js
// ou: frida -n NarakaBladepoint.exe -l entity.js

console.log("[+] Entity.js - Starting...");

// Encontrar o módulo GameAssembly_Super.dll
var gameAssembly = null;
var gameAssemblyBase = null;

try {
    gameAssembly = Process.findModuleByName("GameAssembly_Super.dll");
    if (gameAssembly) {
        gameAssemblyBase = gameAssembly.base;
        console.log("[+] GameAssembly_Super.dll found!");
        console.log("    Base Address: " + gameAssemblyBase);
        console.log("    Size: 0x" + gameAssembly.size.toString(16));
        console.log("    Path: " + gameAssembly.path);
    } else {
        console.log("[-] GameAssembly_Super.dll NOT FOUND!");
    }
} catch (e) {
    console.log("[-] Error finding GameAssembly_Super.dll: " + e);
}

// Verificar se o processo está conectado
try {
    var processName = Process.getCurrentThreadId();
    console.log("[+] Connected to process!");
    console.log("    Current Thread ID: " + processName);
} catch (e) {
    console.log("[-] Error getting process info: " + e);
}

// Listar todos os módulos carregados
console.log("\n[+] Loaded Modules:");
Process.enumerateModules().forEach(function(module) {
    if (module.name.toLowerCase().includes("naraka") || 
        module.name.toLowerCase().includes("game") || 
        module.name.toLowerCase().includes("unity")) {
        console.log("    - " + module.name + " @ " + module.base);
    }
});

// Se encontrou o GameAssembly, calcular endereços dos RVAs
if (gameAssemblyBase) {
    console.log("\n[+] Calculating RVA addresses:");
    
    var rvas = {
        "RVA_GET_ALIVE_CHAR_LIST": 0x570c050,//0x5321aa0,
        "RVA_GET_ALIVE_CHAR_LIST_ALT": 0xcc31810,//0xd9ba1b0,
        "RVA_GET_MAIN_PLAYER": 0xcc2dfa0//0xd9b6a20
    };
    
    for (var rvaName in rvas) {
        var rva = rvas[rvaName];
        var absoluteAddress = gameAssemblyBase.add(rva);
        console.log("    " + rvaName + " (0x" + rva.toString(16) + ")");
        console.log("        -> Absolute: " + absoluteAddress);
    }
    
    console.log("\n[+] Ready to hook!");
    console.log("    Use these addresses in your mod menu:");
    console.log("    GameAssembly_Super.dll base = " + gameAssemblyBase);
} else {
    console.log("\n[-] Cannot calculate addresses - GameAssembly_Super.dll not found!");
}

console.log("\n[+] Entity.js - Loaded successfully!");
console.log("[*] Script is running. Press Ctrl+C to detach.");

// ========================================
// Hook para capturar CharacterManager e contar players vivos
// ========================================

if (gameAssemblyBase) {
    console.log("\n[+] Setting up hooks to capture alive players...");
    
    var characterManagerInstance = null;
    
    // Hook get_AllWaitCleanCharacterList para capturar CharacterManager instance (RVA: 0x4cea940)
    var getAllWaitCleanAddr = gameAssemblyBase.add(0x570c050);//(0x5321aa0);
    console.log("[+] Hooking get_AllWaitCleanCharacterList at: " + getAllWaitCleanAddr);
    
    Interceptor.attach(getAllWaitCleanAddr, {
        onEnter: function(args) {
            // O 'this' pointer (rcx no x64) é a instância do CharacterManager
            var thisPtr = this.context.rcx;
            
            if (!thisPtr.isNull() && !characterManagerInstance) {
                characterManagerInstance = thisPtr;
                console.log("\n[+] CharacterManager instance captured!");
                console.log("    Address: " + characterManagerInstance);
            }
        }
    });
    
    // Função para ler a lista de atores vivos
    function readAliveActorsList() {
        if (!characterManagerInstance) {
            console.log("[-] CharacterManager instance not captured yet. Enter the game!");
            return;
        }
        
        console.log("\n[+] Reading alive actors list...");
        
        try {
            // Chamar get_AllAliveCharacterList (RVA: 0x5321aa0)
            var getAllAliveList = new NativeFunction(
                gameAssemblyBase.add(0x570c050),
                'pointer',
                ['pointer']
            );
            
            var listObj = getAllAliveList(characterManagerInstance);
            
            if (listObj.isNull()) {
                console.log("    RVA 0x5321aa0: NULL");
                return;
            }
            
            // Estrutura IL2CPPList:
            // +0x00: klass
            // +0x08: monitor
            // +0x10: items (ponteiro para IL2CPPArray)
            // +0x18: size (int)
            
            var itemsPtr = listObj.add(0x10).readPointer();  // Ler ponteiro do array
            var size = listObj.add(0x18).readInt();          // Ler size
            
            console.log("    RVA 0x5321aa0: " + size + " actors alive ✓");
            
            if (size == 0 || itemsPtr.isNull()) {
                return;
            }
            
            // Estrutura IL2CPPArray:
            // +0x00: klass
            // +0x08: monitor
            // +0x10: bounds
            // +0x18: max_length
            // +0x20: [elementos começam aqui]
            
            console.log("\n    [+] Reading actor positions:");
            
            // RVAs para funções Unity
            var getTransform = new NativeFunction(
                gameAssemblyBase.add(0x11639430),  // RVA_GET_TRANSFORM
                'pointer',
                ['pointer']
            );
            
            var getPosition = new NativeFunction(
                gameAssemblyBase.add(0x1164db50),  // RVA_GET_POSITION
                'void',
                ['pointer', 'pointer']
            );
            
            // Iterar pelos primeiros 5 atores
            var maxActors = Math.min(size, 5);
            
            for (var i = 0; i < maxActors; i++) {
                try {
                    // Ler ponteiro do ActorModel (cada ponteiro = 8 bytes)
                    var actorPtr = itemsPtr.add(0x20 + (i * 8)).readPointer();
                    
                    if (actorPtr.isNull()) {
                        console.log("        Actor " + i + ": NULL");
                        continue;
                    }
                    
                    // Obter transform diretamente do offset +0x60 no ActorModel
                    var transform = actorPtr.add(0x60).readPointer();
                    
                    if (transform.isNull()) {
                        console.log("        Actor " + i + ": Transform is NULL (offset 0x60)");
                        continue;
                    }
                    
                    // Alocar buffer para Vector3 (12 bytes: float x, y, z)
                    var posBuffer = Memory.alloc(16);  // 16 bytes para alinhamento
                    
                    // Chamar Transform.get_position
                    getPosition(posBuffer, transform);
                    
                    // Ler Vector3
                    var x = posBuffer.readFloat();
                    var y = posBuffer.add(4).readFloat();
                    var z = posBuffer.add(8).readFloat();
                    
                    console.log("        Actor " + i + ": (" + 
                               x.toFixed(2) + ", " + 
                               y.toFixed(2) + ", " + 
                               z.toFixed(2) + ")");
                    
                } catch (e) {
                    console.log("        Actor " + i + ": Error - " + e.message);
                }
            }
            
            if (size > 5) {
                console.log("        ... and " + (size - 5) + " more actors");
            }
            
        } catch (e) {
            console.log("    ERROR: " + e.message);
        }
    }
    
    // Função para ler a estrutura da câmera e encontrar offsets
    this.dumpCameraStructure = function() {
        console.log("\n[+] Dumping Camera structure...");
        
        try {
            var getMainCamera = new NativeFunction(
                gameAssemblyBase.add(0x1160a730),
                'pointer',
                []
            );
            
            var camera = getMainCamera();
            
            if (camera.isNull()) {
                console.log("[-] Camera not found!");
                return;
            }
            
            console.log("[+] Camera address: " + camera);
            
            // Procurar por padrões de ViewMatrix (valores como 1.0, -1.0, 0.0)
            console.log("\n[+] Searching for ViewMatrix patterns (floats near 1.0, -1.0, 0.0):");
            
            for (var offset = 0; offset < 512; offset += 4) {
                try {
                    var f = camera.add(offset).readFloat();
                    
                    // Procurar por valores que parecem componentes de matriz
                    if (Math.abs(f) > 0.9 && Math.abs(f) < 1.1) {
                        // Verificar se há mais valores similares próximos (indica matriz)
                        var f2 = camera.add(offset + 4).readFloat();
                        var f3 = camera.add(offset + 8).readFloat();
                        var f4 = camera.add(offset + 12).readFloat();
                        
                        if (Math.abs(f2) < 0.1 || Math.abs(f2) > 0.9) {
                            console.log("    Offset 0x" + offset.toString(16) + ": " + f + ", " + f2 + ", " + f3 + ", " + f4);
                        }
                    }
                } catch (e) {
                    // Ignorar erros de leitura
                }
            }
            
            // Tentar ler todos os floats nos primeiros 512 bytes
            console.log("\n[+] All floats in first 512 bytes:");
            for (var offset = 0; offset < 512; offset += 4) {
                try {
                    var f = camera.add(offset).readFloat();
                    if (Math.abs(f) > 0.001) {
                        console.log("    Offset 0x" + offset.toString(16).padStart(3, '0') + ": " + f);
                    }
                } catch (e) {
                    // Ignorar
                }
            }
            
        } catch (e) {
            console.log("[-] ERROR: " + e.message);
        }
    }
    
    // Ler lista a cada 3 segundos
    setInterval(function() {
        readAliveActorsList();
    }, 3000);
    
    console.log("[+] Auto-reading alive actors every 3 seconds...");
    console.log("[+] Type dumpCameraStructure() to dump camera structure");
}
