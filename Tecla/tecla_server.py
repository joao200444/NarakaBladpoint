import socket
import time
import ctypes
from ctypes import wintypes

print("=" * 50)
print("    TECLA - Input Helper v1.0 (Python)")
print("=" * 50)
print()

# Constantes do Windows
INPUT_KEYBOARD = 1
KEYEVENTF_SCANCODE = 0x0008
KEYEVENTF_KEYUP = 0x0002

# Estrutura INPUT para SendInput
class KEYBDINPUT(ctypes.Structure):
    _fields_ = [
        ("wVk", wintypes.WORD),
        ("wScan", wintypes.WORD),
        ("dwFlags", wintypes.DWORD),
        ("time", wintypes.DWORD),
        ("dwExtraInfo", ctypes.POINTER(ctypes.c_ulong))
    ]

class INPUT(ctypes.Structure):
    class _INPUT(ctypes.Union):
        _fields_ = [("ki", KEYBDINPUT)]
    _anonymous_ = ("_input",)
    _fields_ = [
        ("type", wintypes.DWORD),
        ("_input", _INPUT)
    ]

# Carregar user32.dll
user32 = ctypes.WinDLL('user32', use_last_error=True)

def press_key_l_spam():
    """Pressiona a tecla L com spam (2 cliques em 1 segundo) - Método AutoHotkey"""
    print("[TECLA] Pressionando tecla L (2 cliques)...")
    
    # Scan code da tecla L = 0x26
    scan_code_l = 0x26
    
    # 2 cliques em 1 segundo
    clicks = 2
    delay_ms = 1000 / clicks  # 500ms entre cada clique
    
    for i in range(clicks):
        # Criar input para DOWN
        input_down = INPUT()
        input_down.type = INPUT_KEYBOARD
        input_down.ki.wVk = 0
        input_down.ki.wScan = scan_code_l
        input_down.ki.dwFlags = KEYEVENTF_SCANCODE
        input_down.ki.time = 0
        input_down.ki.dwExtraInfo = None
        
        # Criar input para UP
        input_up = INPUT()
        input_up.type = INPUT_KEYBOARD
        input_up.ki.wVk = 0
        input_up.ki.wScan = scan_code_l
        input_up.ki.dwFlags = KEYEVENTF_SCANCODE | KEYEVENTF_KEYUP
        input_up.ki.time = 0
        input_up.ki.dwExtraInfo = None
        
        # Enviar DOWN
        user32.SendInput(1, ctypes.byref(input_down), ctypes.sizeof(INPUT))
        
        # Delay pequeno
        time.sleep((delay_ms / 2) / 1000)
        
        # Enviar UP
        user32.SendInput(1, ctypes.byref(input_up), ctypes.sizeof(INPUT))
        
        # Delay até próximo clique
        time.sleep((delay_ms / 2) / 1000)
    
    print(f"[TECLA] {clicks} cliques de L enviados!")

def start_server():
    """Inicia servidor TCP na porta 7777"""
    server = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    server.bind(('127.0.0.1', 7777))
    server.listen(1)
    
    print("[TECLA] Servidor iniciado na porta 7777")
    print("[TECLA] Aguardando conexao da DLL...")
    print()
    
    while True:
        try:
            client, addr = server.accept()
            print(f"[TECLA] Cliente conectado: {addr}")
            
            while True:
                try:
                    data = client.recv(1024)
                    if not data:
                        break
                    
                    command = data.decode('utf-8').strip()
                    print(f"[TECLA] Comando recebido: {command}")
                    
                    if command == "PRESS_L":
                        press_key_l_spam()
                    elif command == "PING":
                        print("[TECLA] PONG")
                    else:
                        print(f"[TECLA] Comando desconhecido: {command}")
                
                except Exception as e:
                    print(f"[ERRO] Erro ao receber dados: {e}")
                    break
            
            client.close()
            print("[TECLA] Cliente desconectado")
            print("[TECLA] Aguardando nova conexao...")
            
        except KeyboardInterrupt:
            print("\n[TECLA] Encerrando servidor...")
            break
        except Exception as e:
            print(f"[ERRO] Erro no servidor: {e}")
    
    server.close()
    print("[TECLA] Servidor encerrado!")

if __name__ == "__main__":
    try:
        start_server()
    except Exception as e:
        print(f"[ERRO FATAL] {e}")
        input("Pressione Enter para sair...")

