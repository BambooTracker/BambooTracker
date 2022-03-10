set (BT_TRANSLATIONS
	bamboo_tracker_ja.ts
	bamboo_tracker_fr.ts
	bamboo_tracker_pl.ts
)

if (QT_VERSION EQUAL 6)
	qt6_add_translation (TRANSLATIONS ${BT_TRANSLATIONS})
else()
	qt5_add_translation (TRANSLATIONS ${BT_TRANSLATIONS})
endif()

add_custom_target (BambooTracker_translations ALL DEPENDS ${TRANSLATIONS})
install (FILES ${TRANSLATIONS} DESTINATION "${BT_LANGDIR}")
