// Frida script para clicar L automaticamente
// Foco: Clicar L continuamente quando o script inicializa
// Uso: frida -l Counter.js NarakaBladepoint.exe

console.log("[*] Counter Script iniciado");
console.log("[!] OBJETIVO: Clicar L automaticamente em loop");

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

// Função para clicar L
function pressL() {
    try {
        // Scan code da tecla L = 0x26
        var scanCodeL = 0x26;
        
        // Key down
        var inputDown = createKeyInput(scanCodeL, false);
        sendInput(1, inputDown, 40);
        
        // Pequeno delay
        Thread.sleep(10);
        
        // Key up
        var inputUp = createKeyInput(scanCodeL, true);
        sendInput(1, inputUp, 40);
        
    } catch(e) {
        console.log("[-] Erro ao pressionar L: " + e);
    }
}

// Loop para clicar L automaticamente
console.log("[+] Iniciando loop de clique L...");
console.log("[!] Pressione Ctrl+C no Frida para parar");

setInterval(function() {
    pressL();
    console.log("[AUTO] L clicado!");
}, 100);  // Clica a cada 100ms (10 cliques por segundo)
