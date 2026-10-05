// Test script para verificar RVA da câmera - get_main
// Uso: frida -l tst.js -n NarakaBladepoint.exe

console.log("[*] Camera Test Script Started");

// Encontrar o módulo GameAssembly_Super.dll
var gameAssembly = Process.findModuleByName("GameAssembly_Super.dll");

if (!gameAssembly) {
    console.log("[-] GameAssembly_Super.dll not found!");
}

console.log("[+] GameAssembly_Super.dll found at: " + gameAssembly.base);
console.log("[+] GameAssembly_Super.dll size: " + gameAssembly.size);

// RVA get_main (novo)
var CAMERA_RVA = 0x1160a730;
var cameraFuncAddr = gameAssembly.base.add(CAMERA_RVA);

console.log("[+] get_main at: " + cameraFuncAddr);

// RVAs para obter transform e posição
var RVA_GET_TRANSFORM = 0x11639430;
var RVA_GET_POSITION = 0x1164db50;

var getTransform = new NativeFunction(
    gameAssembly.base.add(RVA_GET_TRANSFORM),
    'pointer',
    ['pointer']
);

var getPosition = new NativeFunction(
    gameAssembly.base.add(RVA_GET_POSITION),
    'void',
    ['pointer', 'pointer']
);

// Hook get_main
try {
    var callCount = 0;
    Interceptor.attach(cameraFuncAddr, {
        onEnter: function(args) {
            callCount++;
            console.log("[*] get_main called (call #" + callCount + ")");
        },
        onLeave: function(retval) {
            console.log("[+] get_main returned: " + retval);

            if (retval.isNull() || retval.toInt32() === 0 || retval.toInt32() === -1) {
                console.log("[-] Camera returned NULL or invalid");
                return;
            }

            console.log("[+] Camera found at: " + retval);

            // Tentar obter a posição
            try {
                var transform = getTransform(retval);
                if (transform && !transform.isNull() && transform.toInt32() !== 0) {
                    var posBuffer = Memory.alloc(16);
                    getPosition(posBuffer, transform);

                    var x = posBuffer.readFloat();
                    var y = posBuffer.add(4).readFloat();
                    var z = posBuffer.add(8).readFloat();

                    console.log("[+] Camera Position: (" + x.toFixed(2) + ", " + y.toFixed(2) + ", " + z.toFixed(2) + ")");
                } else {
                    console.log("[-] Camera transform is NULL or invalid");
                }
            } catch (e) {
                console.log("[-] Error getting position: " + e);
            }
        }
    });

    console.log("[+] Hooked get_main");
    console.log("[*] Move around in the game to trigger camera updates");
    console.log("[*] Press Ctrl+C to exit");

} catch (e) {
    console.log("[-] Error hooking get_main: " + e);
}
