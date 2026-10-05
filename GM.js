// Frida script - GM Spectator/Referee Vision
// Funções de GM/Admin para visão de espectador/árbitro (ver através de paredes)
// Uso: python -m frida_tools.repl -p <PID> -l GM.js

console.log("[*] GM Spectator/Referee Vision iniciado");

var gameAssembly = Process.findModuleByName("GameAssembly_Super.dll");

if (gameAssembly) {
    console.log("[+] GameAssembly_Super.dll: " + gameAssembly.base);

    // RVAs - Funções Culling (controla o que é renderizado/visível)
    var RVA_ENABLE_CULLING = 0x881c010;              // EnableCulling
    var RVA_NEED_CULLING = 0x881bf80;                 // NeedCulling
    var RVA_REFRESH_CULLING_LOD = 0x87fc390;         // RefreshCullingLodLevel
    var RVA_REFRESH_ACTOR_CULLING = 0x87fb970;        // RefreshActorCullingSetting

    // RVAs - Funções HeadAgent (nome acima da cabeça)
    var RVA_HIDE_ALL_HEAD_AGENTS = 0x8252460;         // HideAllHeadAgents
    var RVA_SHOW_ALL_BATTLE_UI = 0x8252830;           // ShowAllBattleUI
    var RVA_IN_OCCLUSION = 0x9601480;                 // InOcclusion (detecta atrás de objetos)
    var RVA_TOGGLE_PARTNER_RIM = 0x5000160;           // CommandTogglePatrnerRimEffect (contorno amarelo)

    // Hooks para desabilitar Culling e ativar HeadAgent/Rim
    function hookCullingAndHeadAgentFunctions() {
        // Hook NeedCulling - desativa culling para ver através de paredes
        Interceptor.attach(gameAssembly.base.add(RVA_NEED_CULLING), {
            onEnter: function(args) {
                console.log("[CULLING] NeedCulling chamado");
            },
            onLeave: function(retval) {
                console.log("[CULLING] NeedCulling retornou: " + retval + " -> FORÇANDO PARA 0");
                retval.replace(ptr(0)); // Força retorno false
            }
        });

        // Hook InOcclusion - força retorno true para ativar rim effect
        Interceptor.attach(gameAssembly.base.add(RVA_IN_OCCLUSION), {
            onEnter: function(args) {
                console.log("[OCCLUSION] InOcclusion chamado");
            },
            onLeave: function(retval) {
                console.log("[OCCLUSION] InOcclusion retornou: " + retval + " -> FORÇANDO PARA 1");
                retval.replace(ptr(1)); // Força retorno true para ativar contorno
            }
        });

        // Hook TogglePartnerRim - ativa contorno amarelo
        Interceptor.attach(gameAssembly.base.add(RVA_TOGGLE_PARTNER_RIM), {
            onEnter: function(args) {
                console.log("[RIM] CommandTogglePatrnerRimEffect chamado");
                console.log("[RIM] Args[0]: " + args[0] + ", Args[1]: " + args[1]);
            }
        });

        // Hook HideAllHeadAgents - impede de esconder nomes
        Interceptor.attach(gameAssembly.base.add(RVA_HIDE_ALL_HEAD_AGENTS), {
            onEnter: function(args) {
                console.log("[HEAD] HideAllHeadAgents chamado - BLOQUEANDO");
                this.ret = 0; // Bloqueia a execução
            }
        });

        // Hook ShowAllBattleUI - monitora
        Interceptor.attach(gameAssembly.base.add(RVA_SHOW_ALL_BATTLE_UI), {
            onEnter: function(args) {
                console.log("[HEAD] ShowAllBattleUI chamado");
            }
        });

        console.log("[+] Hooks Culling e HeadAgent instalados");
    }

    // Ativar hooks automaticamente ao carregar
    function enableGMFunctions() {
        console.log("\n[*] Ativando funções GM Wallhack + HeadAgent + Rim...");
        hookCullingAndHeadAgentFunctions();
        console.log("[+] Hooks ativados automaticamente!");
    }

    // Exportar funções
    this.enableGMFunctions = enableGMFunctions;

    // Ativar automaticamente ao carregar
    enableGMFunctions();

    console.log("\n[*] Script carregado e hooks ativados!");
    console.log("[*] Wallhack: Culling desativado");
    console.log("[*] HeadAgent: Nomes acima das cabeças visíveis");
    console.log("[*] Rim: Contorno amarelo quando atrás de objetos");

} else {
    console.log("[-] GameAssembly_Super.dll não encontrado!");
}
