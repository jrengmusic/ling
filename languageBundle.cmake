#[[*****************************************************************************
                        Codegen Annotated Source of Truth
————————————————————————————————————————————————————————————————————————————————

            ░░████████████░░████████████░░████████████░░████████████
            ░░████  ░░████░░████  ░░████░░████  ░░████    ░░████
            ░░████        ░░████  ░░████░░████            ░░████
            ░░████        ░░████████████░░████████████    ░░████
            ░░████        ░░████  ░░████        ░░████    ░░████
            ░░████  ░░████░░████  ░░████░░████  ░░████    ░░████
            ░░████████████░░████  ░░████░░████████████    ░░████

————————————————————————————————————————————————————————————————————————————————
                         FOR YOUR EYES ONLY, DO NOT EDIT
******************************************************************************]]

if(APPLE)
    set(CAST_PACK_folder "../Release")
elseif(WIN32)
    set(CAST_PACK_folder "../Release")
endif()
if(APPLE)
    set(CAST_PACK_format "pkg")
elseif(WIN32)
    set(CAST_PACK_format "exe")
endif()
if(APPLE)
    set(CAST_PACK_platform "macOS")
elseif(WIN32)
    set(CAST_PACK_platform "Windows")
endif()

get_filename_component(CAST_CORE_FOLDER "${CMAKE_CURRENT_LIST_DIR}/language/core/spell.md" DIRECTORY)
get_filename_component(CAST_CORE_NAME "${CAST_CORE_FOLDER}" NAME)
set(CAST_CORE_BUILD "${CMAKE_CURRENT_LIST_DIR}/Builds/languages/${CAST_CORE_NAME}")
get_filename_component(CAST_LANGUAGE_FOLDER "${CMAKE_CURRENT_LIST_DIR}/language/cLang/spell.md" DIRECTORY)
get_filename_component(CAST_LANGUAGE_NAME "${CAST_LANGUAGE_FOLDER}" NAME)
set(CAST_LANGUAGE_BUILD "${CMAKE_CURRENT_LIST_DIR}/Builds/languages/${CAST_LANGUAGE_NAME}")
set(CAST_CORE_HEADER "ling_LanguageCore.h")
list(TRANSFORM CAST_LANE PREPEND "--" OUTPUT_VARIABLE CAST_LANGUAGE_LANE)

if(APPLE)
    set(CAST_CORE_FILE "build/liblanguageCore.a")
    set(CAST_LANGUAGE_FILE "build/liblanguage.dylib")
elseif(WIN32)
    set(CAST_CORE_FILE "build/languageCore.lib")
    set(CAST_LANGUAGE_FILE "build/language.dll")
endif()

file(COPY "${CAST_CORE_FOLDER}/" DESTINATION "${CAST_CORE_BUILD}")
file(COPY "${CMAKE_CURRENT_LIST_DIR}/../jam/jam_syntax_tree/document/jam_LanguageInterface.h" "${CMAKE_CURRENT_LIST_DIR}/Source/generated/ProjectInfo.h" DESTINATION "${CAST_CORE_BUILD}")
execute_process(COMMAND cast spell.md ${CAST_LANGUAGE_LANE} WORKING_DIRECTORY "${CAST_CORE_BUILD}" COMMAND_ERROR_IS_FATAL ANY)
execute_process(COMMAND cast --pack "${CMAKE_CURRENT_LIST_DIR}/Source/languages/${CAST_PACK_platform}/${CAST_CORE_NAME}.zip" "${CAST_CORE_BUILD}/${CAST_CORE_HEADER}" "${CAST_CORE_BUILD}/${CAST_CORE_FILE}" COMMAND_ERROR_IS_FATAL ANY)

file(COPY "${CAST_LANGUAGE_FOLDER}/" DESTINATION "${CAST_LANGUAGE_BUILD}")
file(COPY "${CMAKE_CURRENT_LIST_DIR}/../jam/cast/core.md" "${CMAKE_CURRENT_LIST_DIR}/../jam/cast/syntax_tree.md" "${CMAKE_CURRENT_LIST_DIR}/cast/lsp.md" "${CMAKE_CURRENT_LIST_DIR}/../jam/jam_syntax_tree/document/jam_LanguageInterface.h" "${CAST_CORE_BUILD}/${CAST_CORE_HEADER}" "${CAST_CORE_BUILD}/${CAST_CORE_FILE}" DESTINATION "${CAST_LANGUAGE_BUILD}")

execute_process(COMMAND cast spell.md ${CAST_LANGUAGE_LANE} WORKING_DIRECTORY "${CAST_LANGUAGE_BUILD}" COMMAND_ERROR_IS_FATAL ANY)
execute_process(COMMAND cast --pack "${CMAKE_CURRENT_LIST_DIR}/Source/languages/${CAST_PACK_platform}/${CAST_LANGUAGE_NAME}.zip" "${CAST_LANGUAGE_BUILD}/${CAST_LANGUAGE_FILE}" COMMAND_ERROR_IS_FATAL ANY)
