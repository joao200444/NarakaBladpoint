// Frida script - SISTEMA COMPLETO DE TRIAL (Unlock TODAS as skins)
// Solução: Forçar IsResourceTrialing() a retornar TRUE para TODOS os recursos
// Isso ativa o sistema completo de trial/test do modo treinamento!
// Uso: python -m frida_tools.repl -p <PID> -l frida_training_mode_unlock.js

console.log("[*] Trial System Complete Unlock Script iniciado");

var gameAssembly = Process.findModuleByName("GameAssembly_Super.dll");

if (gameAssembly) {
    console.log("[+] GameAssembly_Super.dll: " + gameAssembly.base);
    
    // RVAs - Sistema de Trial
    var RVA_IS_RESOURCE_TRIALING = 0x85ec6f0;            // UserDataTrialResource.IsResourceTrialing
    var RVA_IS_RESOURCE_AVAILABLE = 0x970f230;           // UserDataTrialResource.IsResourceAvailable
    var RVA_GET_TRIAL_RESOURCE_INFO = 0x9788370;         // UserDataTrialResource.GetTrialResourceInfo

    // RVAs de unlock (para remover cadeados)
    var RVA_IS_SKIN_OWN = 0xbdf9d40;                     // BattleShowCustom.IsSkinOwn
    var RVA_IS_WEAPON_SKIN_OWN = 0x798bb60;              // Arsenal.IsWeaponSkinOwn
    var RVA_IS_WEAPON_TYPE_OWN = 0xb8cac60;              // Arsenal.IsWeaponTypeOwn
    var RVA_CAN_USER_WEAPON_SKIN = 0xb8cabc0;            // Arsenal.CanUserWeaponSkin
    var RVA_IS_HERO_OWNED = 0x920cb90;                   // HeroData.IsHeroOwned
    var RVA_IS_FASHION_BODY_OWNED = 0xc016ca0;           // FashionData.IsFashionBodyOwned
    var RVA_IS_EMOTE_OWNED = 0x97f7070;                  // EmoteData.IsEmoteOwned
    var RVA_IS_ORNAMENT_OWNED = 0x985c730;               // OrnamentData.IsOrnamentOwned
    var RVA_IS_SOUL_GRAVE_SKIN_OWN = 0xbdf9ed0;          // BattleShowCustom.IsSoulGraveSkinOwn
    var RVA_IS_RESCUE_SKIN_OWN = 0xbdfa000;              // BattleShowCustom.IsRescueSkinOwn
    var RVA_IS_KILL_BROADCAST_SKIN_OWN = 0xbdfa1d0;      // BattleShowCustom.IsKillBroadcastSkinOwn
    var RVA_IS_TREASURE_CHEST_SKIN_OWN = 0xbdfa3a0;      // BattleShowCustom.IsTreasureChestSkinOwn
    
    var addr_is_resource_trialing = gameAssembly.base.add(RVA_IS_RESOURCE_TRIALING);
    var addr_is_resource_available = gameAssembly.base.add(RVA_IS_RESOURCE_AVAILABLE);
    
    console.log("[+] IsResourceTrialing: " + addr_is_resource_trialing);
    console.log("[+] IsResourceAvailable: " + addr_is_resource_available);
    
    var hooksInstalled = false;
    
    // Hook IsResourceTrialing - SEMPRE RETORNA TRUE (todos os recursos em trial)
    function hookIsResourceTrialing() {
        Interceptor.replace(addr_is_resource_trialing, new NativeCallback(function(thisPtr, resourceType, resourceSourceID) {
            // SEMPRE TRUE - todos os recursos estão em trial
            console.log("[+] IsResourceTrialing(" + resourceType + ", " + resourceSourceID + ") = TRUE (FORÇADO)");
            return 1;
        }, 'int', ['pointer', 'int', 'int']));
        
        console.log("[+] Hook IsResourceTrialing instalado (SEMPRE TRUE)");
    }
    
    // Hook IsResourceAvailable - SEMPRE RETORNA TRUE (todos os recursos disponíveis)
    function hookIsResourceAvailable() {
        Interceptor.replace(addr_is_resource_available, new NativeCallback(function(thisPtr, resourceType, resourceSourceID) {
            // SEMPRE TRUE - todos os recursos disponíveis
            console.log("[+] IsResourceAvailable(" + resourceType + ", " + resourceSourceID + ") = TRUE (FORÇADO)");
            return 1;
        }, 'int', ['pointer', 'int', 'int']));
        
        console.log("[+] Hook IsResourceAvailable instalado (SEMPRE TRUE)");
    }
    
    // Hooks para REMOVER CADEADOS (12 hooks)
    function hookUnlockAll() {
        if (hooksInstalled) {
            console.log("[!] Hooks já instalados, pulando...");
            return;
        }
        
        // Battle Show Custom (5 hooks)
        Interceptor.replace(gameAssembly.base.add(RVA_IS_SKIN_OWN), new NativeCallback(function() { return 1; }, 'int', ['pointer', 'int']));
        Interceptor.replace(gameAssembly.base.add(RVA_IS_SOUL_GRAVE_SKIN_OWN), new NativeCallback(function() { return 1; }, 'int', ['pointer', 'int']));
        Interceptor.replace(gameAssembly.base.add(RVA_IS_RESCUE_SKIN_OWN), new NativeCallback(function() { return 1; }, 'int', ['pointer', 'int']));
        Interceptor.replace(gameAssembly.base.add(RVA_IS_KILL_BROADCAST_SKIN_OWN), new NativeCallback(function() { return 1; }, 'int', ['pointer', 'int']));
        Interceptor.replace(gameAssembly.base.add(RVA_IS_TREASURE_CHEST_SKIN_OWN), new NativeCallback(function() { return 1; }, 'int', ['pointer', 'int']));
        
        // Arsenal/Weapon Skins (3 hooks)
        Interceptor.replace(gameAssembly.base.add(RVA_IS_WEAPON_TYPE_OWN), new NativeCallback(function() { return 1; }, 'int', ['pointer', 'int']));
        Interceptor.replace(gameAssembly.base.add(RVA_IS_WEAPON_SKIN_OWN), new NativeCallback(function() { return 1; }, 'int', ['pointer', 'int']));
        Interceptor.replace(gameAssembly.base.add(RVA_CAN_USER_WEAPON_SKIN), new NativeCallback(function() { return 1; }, 'int', ['pointer', 'int']));
        
        // Hero/Fashion (4 hooks)
        Interceptor.replace(gameAssembly.base.add(RVA_IS_HERO_OWNED), new NativeCallback(function() { return 1; }, 'int', ['pointer', 'int']));
        Interceptor.replace(gameAssembly.base.add(RVA_IS_FASHION_BODY_OWNED), new NativeCallback(function() { return 1; }, 'int', ['pointer', 'int']));
        Interceptor.replace(gameAssembly.base.add(RVA_IS_EMOTE_OWNED), new NativeCallback(function() { return 1; }, 'int', ['pointer', 'int']));
        Interceptor.replace(gameAssembly.base.add(RVA_IS_ORNAMENT_OWNED), new NativeCallback(function() { return 1; }, 'int', ['pointer', 'int']));
        
        hooksInstalled = true;
        console.log("[+] 12 hooks de unlock instalados (cadeados removidos)");
    }
    
    // Ativar sistema completo de trial
    function enableTrialSystem() {
        console.log("\n[*] Ativando SISTEMA COMPLETO DE TRIAL...");
        
        hookIsResourceTrialing();    // Força todos os recursos a estarem em trial
        hookIsResourceAvailable();   // Força todos os recursos a estarem disponíveis
        hookUnlockAll();             // Remove cadeados
        
        console.log("\n[+] ✓✓✓ SISTEMA DE TRIAL ATIVADO! ✓✓✓");
        console.log("\n[!] COMO USAR:");
        console.log("  1. Vá para customização de armas/heróis");
        console.log("  2. TODAS as skins estarão em modo TRIAL");
        console.log("  3. Clique em qualquer skin");
        console.log("  4. O botão 'TRIAL' (teste) aparecerá");
        console.log("  5. Clique em 'TRIAL' para equipar");
        console.log("  6. Entre em partida!");
        console.log("\n[!] Sistema completo de trial ativado!");
        console.log("[!] Funciona EXATAMENTE como no modo treinamento!");
    }
    
    // Exportar funções
    this.enableTrialSystem = enableTrialSystem;
    
    console.log("\n[*] Funções disponíveis:");
    console.log("  - enableTrialSystem()  : Ativa sistema completo de trial");
    console.log("\n[+] Script carregado!");
    console.log("[!] Execute: enableTrialSystem()");
    
} else {
    console.log("[-] GameAssembly_Super.dll não encontrado!");
}
