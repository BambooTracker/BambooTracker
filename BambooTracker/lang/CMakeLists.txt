qt5_add_translation(TRANSLATIONS
	bamboo_tracker_ja.ts
	bamboo_tracker_fr.ts
	bamboo_tracker_pl.ts
)
add_custom_target(BambooTracker_translations ALL DEPENDS ${TRANSLATIONS})
install(FILES ${TRANSLATIONS} DESTINATION "${CMAKE_INSTALL_DATADIR}/BambooTracker/lang")
