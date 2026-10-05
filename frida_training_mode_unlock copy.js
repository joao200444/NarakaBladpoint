// Frida script - SISTEMA DE TRIAL (Modo Treinamento no Armário)
// Solução: Forçar o armário a usar o sistema de TRIAL do modo treinamento
// Isso faz todas as skins aparecerem com botão TRIAL e equiparem quando clicadas
// Uso: python -m frida_tools.repl -p <PID> -l frida_training_mode_unlock.js

console.log("[*] Trial System - Armário em Modo Treinamento iniciado");

var gameAssembly = Process.findModuleByName("GameAssembly_Super.dll");

if (gameAssembly) {
    console.log("[+] GameAssembly_Super.dll: " + gameAssembly.base);

    // RVAs - Sistema de Trial (modo treinamento - armário)
    var RVA_GET_PRACTICE_FASHION_TRIALING = 0xc01e500;   // GetPracticeFashionTrialingForSuit
    var RVA_IS_SUIT_FASHION_TRIALING = 0x97ca1c0;         // IsSuitFashionTrialing
    var RVA_IS_SUIT_WEAPON_SKIN_TRIALING = 0x97ca320;     // IsSuitWeaponSkinTrialing
    var RVA_GET_TRIAL_WEAPON_SKIN_ID = 0x99c3ee0;         // GetTrialWeaponSkinId

    // RVAs - UserData e ArsenalData
    var RVA_USER_DATA_INSTANCE = 0x4fca9e0;              // UserData.Instance
    var RVA_WEAPON_SKIN_REQUEST = 0xb8ca8a0;              // UserDataArsenalData.SendUseWeaponSkinRequest (método de instância)
    var RVA_TRIAL_TARGET_WEAPON_SKIN = 0x98db840;         // UserDataArsenalData.TrialTargetWeaponSkinInSuit (método de trial)

    // RVAs - Forçar modo treinamento (Practice Room)
    var RVA_IS_PRACTICE_TYPE = 0xbc5bd50;                // IsPracticeType - Verifica se é modo practice
    var RVA_IS_PRACTICE_MATCH_BATTLE = 0x9e5d910;        // IsPracticeMatchBattle - Verifica se é battle de practice
    var RVA_GET_IS_PRACTICE_ROOM = 0x91a66f0;             // GetIsPracticeRoom - Retorna se está na practice room
    var RVA_IS_EVACUATE_PRACTICE_TYPE = 0xbc5bc80;        // IsEvacuatePracticeType - Verifica se é practice de evacuação

    // RVAs - Botão TRIAL (Trial Button)
    var RVA_REFRESH_TRIAL_BTN = 0x982eb80;               // RefreshTrialBtn - Atualiza botão de trial
    var RVA_SHOW_TRIALING = 0x98d2d30;                   // ShowTrialing - Mostra estado de trialing
    var RVA_REFRESH_TRIAL_BTN_2 = 0x98c2ed0;              // RefreshTrialBtn (segundo)
    var RVA_ON_PRACTICE_TRIAL_INFO_CHANGE = 0x98d9380;    // OnPracticeTrialInfoChange - Quando info de trial muda
    var RVA_ON_BTN_TRIAL_CLICK = 0x98d1f90;                // OnNode_Btns_Btn_TrialClick - Quando clica no botão TRIAL
    var RVA_ON_BTN_PRACTICE_TRIAL_CLICK = 0x98d90e0;       // OnNode_Btns_Btn_Practice_TrialClick - Quando clica no botão TRIAL do practice
    var RVA_ON_BTN_USE_TRIAL_CLICK = 0x985b450;            // OnNode_Config_Btn_Use_TrialClick - Quando clica em Use Trial

    // Hook IsPracticeType - força o jogo a pensar que está no modo practice
    function hookIsPracticeType() {
        Interceptor.replace(gameAssembly.base.add(RVA_IS_PRACTICE_TYPE), new NativeCallback(function() { return 1; }, 'int', ['pointer']));
        console.log("[+] Hook IsPracticeType instalado (SEMPRE TRUE - Modo Practice)");
    }

    // Hook IsPracticeMatchBattle - força o jogo a pensar que é battle de practice
    function hookIsPracticeMatchBattle() {
        Interceptor.replace(gameAssembly.base.add(RVA_IS_PRACTICE_MATCH_BATTLE), new NativeCallback(function() { return 1; }, 'int', ['pointer']));
        console.log("[+] Hook IsPracticeMatchBattle instalado (SEMPRE TRUE)");
    }

    // Hook GetIsPracticeRoom - força o jogo a pensar que está na practice room
    function hookGetIsPracticeRoom() {
        Interceptor.replace(gameAssembly.base.add(RVA_GET_IS_PRACTICE_ROOM), new NativeCallback(function() { return 1; }, 'int', ['pointer']));
        console.log("[+] Hook GetIsPracticeRoom instalado (SEMPRE TRUE - Practice Room)");
    }

    // Hook IsEvacuatePracticeType - força o jogo a pensar que é practice de evacuação
    function hookIsEvacuatePracticeType() {
        Interceptor.replace(gameAssembly.base.add(RVA_IS_EVACUATE_PRACTICE_TYPE), new NativeCallback(function() { return 1; }, 'int', ['pointer']));
        console.log("[+] Hook IsEvacuatePracticeType instalado (SEMPRE TRUE)");
    }

    // Hook RefreshTrialBtn - força atualização do botão de trial
    function hookRefreshTrialBtn() {
        Interceptor.attach(gameAssembly.base.add(RVA_REFRESH_TRIAL_BTN), {
            onEnter: function(args) {
                console.log("[TRIAL_BTN] RefreshTrialBtn chamado");
            }
        });
        console.log("[+] Hook RefreshTrialBtn instalado");
    }

    // Hook ShowTrialing - força mostrar estado de trialing (SEMPRE TRUE)
    function hookShowTrialing() {
        Interceptor.replace(gameAssembly.base.add(RVA_SHOW_TRIALING), new NativeCallback(function() { return 1; }, 'int', ['pointer']));
        console.log("[+] Hook ShowTrialing instalado (SEMPRE TRUE - Botão TRIAL visível)");
    }

    // Hook RefreshTrialBtn_2 - segundo refresh do botão de trial
    function hookRefreshTrialBtn2() {
        Interceptor.attach(gameAssembly.base.add(RVA_REFRESH_TRIAL_BTN_2), {
            onEnter: function(args) {
                console.log("[TRIAL_BTN] RefreshTrialBtn_2 chamado");
            }
        });
        console.log("[+] Hook RefreshTrialBtn_2 instalado");
    }

    // Hook OnNode_Btns_Btn_TrialClick - quando clica no botão TRIAL
    function hookOnBtnTrialClick() {
        Interceptor.attach(gameAssembly.base.add(RVA_ON_BTN_TRIAL_CLICK), {
            onEnter: function(args) {
                console.log("[TRIAL_CLICK] OnNode_Btns_Btn_TrialClick chamado - Botão TRIAL clicado!");

                // Tenta pegar o ID da skin dos argumentos
                var thisPtr = args[0];
                console.log("[TRIAL_CLICK] This pointer: " + thisPtr);

                // Tenta ler o ID da skin em diferentes offsets
                var offsets = [0x10, 0x18, 0x20, 0x28, 0x30, 0x38, 0x40, 0x48, 0x50, 0x58, 0x60, 0x68, 0x70, 0x78, 0x80];

                for (var i = 0; i < offsets.length; i++) {
                    try {
                        var skinId = thisPtr.add(offsets[i]).readInt();
                        if (skinId > 0 && skinId < 10000000) {
                            console.log("[TRIAL_CLICK] Offset " + offsets[i] + ": Skin ID encontrado: " + skinId);
                            console.log("[TRIAL_CLICK] Armazenando skin ID para equipar: " + skinId);
                            // Armazena o skin ID globalmente para usar no hook de SendUseWeaponSkinRequest
                            globalTrialSkinId = skinId;

                            // Tenta chamar SendUseWeaponSkinRequest diretamente
                            try {
                                var userDataInstance = new NativeFunction(gameAssembly.base.add(RVA_USER_DATA_INSTANCE), 'pointer', []);
                                var userData = userDataInstance();
                                if (!userData.isNull()) {
                                    var arsenalData = userData.add(0x148).readPointer();
                                    if (!arsenalData.isNull()) {
                                        console.log("[TRIAL_CLICK] Chamando SendUseWeaponSkinRequest diretamente...");
                                        var sendUseWeaponSkinRequest = new NativeFunction(gameAssembly.base.add(RVA_WEAPON_SKIN_REQUEST), 'void', ['pointer', 'int']);
                                        sendUseWeaponSkinRequest(arsenalData, skinId);
                                        console.log("[TRIAL_CLICK] SendUseWeaponSkinRequest chamado!");
                                    }
                                }
                            } catch (e) {
                                console.log("[TRIAL_CLICK] Erro ao chamar SendUseWeaponSkinRequest: " + e);
                            }
                            return;
                        }
                    } catch (e) {
                        // Ignora erros de leitura
                    }
                }
            }
        });
        console.log("[+] Hook OnNode_Btns_Btn_TrialClick instalado");
    }

    // Hook SendUseWeaponSkinRequest para interceptar e forçar equipamento
    var globalTrialSkinId = null;

    function hookSendUseWeaponSkinRequest() {
        Interceptor.attach(gameAssembly.base.add(RVA_WEAPON_SKIN_REQUEST), {
            onEnter: function(args) {
                var thisPtr = args[0];
                var weaponSkinId = args[1];

                console.log("[WEAPON_SKIN] SendUseWeaponSkinRequest chamado");
                console.log("[WEAPON_SKIN] This pointer: " + thisPtr);
                console.log("[WEAPON_SKIN] WeaponSkinID: " + weaponSkinId);

                // Se temos um skin ID de TRIAL armazenado, usa ele
                if (globalTrialSkinId !== null) {
                    console.log("[WEAPON_SKIN] Substituindo WeaponSkinID por TRIAL ID: " + globalTrialSkinId);
                    args[1] = ptr(globalTrialSkinId);
                    globalTrialSkinId = null; // Limpa após usar
                }
            }
        });
        console.log("[+] Hook SendUseWeaponSkinRequest instalado");
    }

    // Hook TrialTargetWeaponSkinInSuit - método específico para trial de skins
    function hookTrialTargetWeaponSkinInSuit() {
        Interceptor.attach(gameAssembly.base.add(RVA_TRIAL_TARGET_WEAPON_SKIN), {
            onEnter: function(args) {
                var thisPtr = args[0];
                console.log("[TRIAL_TARGET] TrialTargetWeaponSkinInSuit chamado");
                console.log("[TRIAL_TARGET] This pointer: " + thisPtr);
                console.log("[TRIAL_TARGET] Args[1]: " + args[1]);
                console.log("[TRIAL_TARGET] Args[2]: " + args[2]);
                console.log("[TRIAL_TARGET] Args[3]: " + args[3]);
            }
        });
        console.log("[+] Hook TrialTargetWeaponSkinInSuit instalado");
    }

    // Hook OnNode_Btns_Btn_Practice_TrialClick - quando clica no botão TRIAL do practice
    function hookOnBtnPracticeTrialClick() {
        Interceptor.attach(gameAssembly.base.add(RVA_ON_BTN_PRACTICE_TRIAL_CLICK), {
            onEnter: function(args) {
                console.log("[TRIAL_CLICK] OnNode_Btns_Btn_Practice_TrialClick chamado - Botão TRIAL Practice clicado!");
            }
        });
        console.log("[+] Hook OnNode_Btns_Btn_Practice_TrialClick instalado");
    }

    // Hook OnNode_Config_Btn_Use_TrialClick - quando clica em Use Trial
    function hookOnBtnUseTrialClick() {
        Interceptor.attach(gameAssembly.base.add(RVA_ON_BTN_USE_TRIAL_CLICK), {
            onEnter: function(args) {
                console.log("[TRIAL_CLICK] OnNode_Config_Btn_Use_TrialClick chamado - Use Trial clicado!");
            }
        });
        console.log("[+] Hook OnNode_Config_Btn_Use_TrialClick instalado");
    }

    // Hook GetPracticeFashionTrialingForSuit - retorna fashion em trial para o suit
    function hookGetPracticeFashionTrialing() {
        Interceptor.replace(gameAssembly.base.add(RVA_GET_PRACTICE_FASHION_TRIALING), new NativeCallback(function() { return 1; }, 'int', ['pointer', 'int']));
        console.log("[+] Hook GetPracticeFashionTrialingForSuit instalado (SEMPRE TRUE)");
    }

    // Hook IsSuitFashionTrialing - força fashion a estar em trial
    function hookIsSuitFashionTrialing() {
        Interceptor.replace(gameAssembly.base.add(RVA_IS_SUIT_FASHION_TRIALING), new NativeCallback(function() { return 1; }, 'int', ['pointer', 'int']));
        console.log("[+] Hook IsSuitFashionTrialing instalado (SEMPRE TRUE)");
    }

    // Hook IsSuitWeaponSkinTrialing - força skin de arma a estar em trial
    function hookIsSuitWeaponSkinTrialing() {
        Interceptor.replace(gameAssembly.base.add(RVA_IS_SUIT_WEAPON_SKIN_TRIALING), new NativeCallback(function() { return 1; }, 'int', ['pointer', 'int']));
        console.log("[+] Hook IsSuitWeaponSkinTrialing instalado (SEMPRE TRUE)");
    }

    // Ativar sistema de TRIAL (modo treinamento)
    function enableTrialSystem() {
        console.log("\n[*] Ativando SISTEMA DE TRIAL (Armário do Treinamento)...");

        // Força modo treinamento
        hookIsPracticeType();           // Força modo practice
        hookIsPracticeMatchBattle();    // Força battle de practice
        hookGetIsPracticeRoom();        // Força practice room
        hookIsEvacuatePracticeType();   // Força practice de evacuação

        // Força botão TRIAL
        hookRefreshTrialBtn();          // Hooka atualização do botão trial
        hookShowTrialing();             // Hooka mostrar trialing
        hookRefreshTrialBtn2();         // Hooka segundo refresh do botão trial

        // Hook clique no botão TRIAL
        hookOnBtnTrialClick();          // Hooka clique no botão TRIAL
        hookOnBtnPracticeTrialClick();  // Hooka clique no botão TRIAL do practice
        hookOnBtnUseTrialClick();       // Hooka clique em Use Trial

        // Força trial de skins
        hookGetPracticeFashionTrialing();
        hookIsSuitFashionTrialing();
        hookIsSuitWeaponSkinTrialing();

        // Hook equipamento de skins
        hookSendUseWeaponSkinRequest();
        hookTrialTargetWeaponSkinInSuit();

        console.log("[+] Sistema de TRIAL ativado!");
        console.log("[+] Entre no armário e você verá o botão TRIAL em todas as skins");
    }

    // Exportar funções
    this.enableTrialSystem = enableTrialSystem;

    console.log("\n[*] Funções disponíveis:");
    console.log("  - enableTrialSystem()  : Ativa sistema de TRIAL (armário do treinamento)");
    console.log("\n[+] Script carregado!");
    console.log("[!] Execute: enableTrialSystem()");
    
} else {
    console.log("[-] GameAssembly_Super.dll não encontrado!");
}
