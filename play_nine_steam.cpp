struct Steam_Manager {
  u64 lobby_id;
  
  STEAM_CALLBACK(Steam_Manager, on_game_overlay_activated, GameOverlayActivated_t);
  STEAM_CALLBACK(Steam_Manager, lobby_game_created, LobbyGameCreated_t);

  void create_lobby(ELobbyType eLobbyType, int cMaxMembers);

  void on_lobby_created(LobbyCreated_t *pCallback, bool bIOFailure);
  CCallResult<Steam_Manager, LobbyCreated_t> m_LobbyCreatedCallResult;
};

Steam_Manager *steam_manager;

void Steam_Manager::on_game_overlay_activated(GameOverlayActivated_t *pCallback) {
  State *state = (State *)app.data;

  if (pCallback->m_bActive) {
      print("Steam overlay now active\n");

    if (state->menu_list.mode == IN_GAME) {
      state->menu_list.mode = PAUSE_MENU;
    }      
  } else {
      print("Steam overlay now inactive\n");
  }
}

void Steam_Manager::lobby_game_created(LobbyGameCreated_t *p_callback) {
  
}

void Steam_Manager::create_lobby(ELobbyType eLobbyType, int cMaxMembers)  {
  SteamAPICall_t h_steam_api_call = SteamMatchmaking()->CreateLobby(eLobbyType, cMaxMembers);
  m_LobbyCreatedCallResult.Set(h_steam_api_call, this, &Steam_Manager::on_lobby_created);
}

void Steam_Manager::on_lobby_created(LobbyCreated_t *pCallback, bool bIOFailure) {
    if (bIOFailure || pCallback->m_eResult != k_EResultOK) {
      logprint("on_lobby_created", "LobbyCreate_t failed\n");
      return;
    }

    print("(steam) Lobby Created");

    lobby_id = pCallback->m_ulSteamIDLobby;
}
