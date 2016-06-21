set(
    HEADERS
    ${CMAKE_CURRENT_LIST_DIR}/alias.hpp
    ${CMAKE_CURRENT_LIST_DIR}/connection.hpp
    ${CMAKE_CURRENT_LIST_DIR}/cmd-help.hpp
    ${CMAKE_CURRENT_LIST_DIR}/cmd-plugin-config.hpp
    ${CMAKE_CURRENT_LIST_DIR}/cmd-plugin-info.hpp
    ${CMAKE_CURRENT_LIST_DIR}/cmd-plugin-list.hpp
    ${CMAKE_CURRENT_LIST_DIR}/cmd-plugin-load.hpp
    ${CMAKE_CURRENT_LIST_DIR}/cmd-plugin-reload.hpp
    ${CMAKE_CURRENT_LIST_DIR}/cmd-plugin-unload.hpp
    ${CMAKE_CURRENT_LIST_DIR}/cmd-server-cmode.hpp
    ${CMAKE_CURRENT_LIST_DIR}/cmd-server-cnotice.hpp
    ${CMAKE_CURRENT_LIST_DIR}/cmd-server-connect.hpp
    ${CMAKE_CURRENT_LIST_DIR}/cmd-server-disconnect.hpp
    ${CMAKE_CURRENT_LIST_DIR}/cmd-server-info.hpp
    ${CMAKE_CURRENT_LIST_DIR}/cmd-server-invite.hpp
    ${CMAKE_CURRENT_LIST_DIR}/cmd-server-join.hpp
    ${CMAKE_CURRENT_LIST_DIR}/cmd-server-kick.hpp
    ${CMAKE_CURRENT_LIST_DIR}/cmd-server-list.hpp
    ${CMAKE_CURRENT_LIST_DIR}/cmd-server-me.hpp
    ${CMAKE_CURRENT_LIST_DIR}/cmd-server-message.hpp
    ${CMAKE_CURRENT_LIST_DIR}/cmd-server-mode.hpp
    ${CMAKE_CURRENT_LIST_DIR}/cmd-server-nick.hpp
    ${CMAKE_CURRENT_LIST_DIR}/cmd-server-notice.hpp
    ${CMAKE_CURRENT_LIST_DIR}/cmd-server-part.hpp
    ${CMAKE_CURRENT_LIST_DIR}/cmd-server-reconnect.hpp
    ${CMAKE_CURRENT_LIST_DIR}/cmd-server-topic.hpp
    ${CMAKE_CURRENT_LIST_DIR}/cmd-watch.hpp
    ${CMAKE_CURRENT_LIST_DIR}/command.hpp
    ${CMAKE_CURRENT_LIST_DIR}/config.hpp
    ${CMAKE_CURRENT_LIST_DIR}/elapsed-timer.hpp
    ${CMAKE_CURRENT_LIST_DIR}/fs.hpp
    ${CMAKE_CURRENT_LIST_DIR}/ini.hpp
    ${CMAKE_CURRENT_LIST_DIR}/irccd.hpp
    ${CMAKE_CURRENT_LIST_DIR}/irccdctl.hpp
    ${CMAKE_CURRENT_LIST_DIR}/json.hpp
    ${CMAKE_CURRENT_LIST_DIR}/mod-directory.hpp
    ${CMAKE_CURRENT_LIST_DIR}/mod-elapsed-timer.hpp
    ${CMAKE_CURRENT_LIST_DIR}/mod-file.hpp
    ${CMAKE_CURRENT_LIST_DIR}/mod-irccd.hpp
    ${CMAKE_CURRENT_LIST_DIR}/mod-logger.hpp
    ${CMAKE_CURRENT_LIST_DIR}/mod-plugin.hpp
    ${CMAKE_CURRENT_LIST_DIR}/mod-server.hpp
    ${CMAKE_CURRENT_LIST_DIR}/mod-system.hpp
    ${CMAKE_CURRENT_LIST_DIR}/mod-timer.hpp
    ${CMAKE_CURRENT_LIST_DIR}/mod-unicode.hpp
    ${CMAKE_CURRENT_LIST_DIR}/mod-util.hpp
    ${CMAKE_CURRENT_LIST_DIR}/net.hpp
    ${CMAKE_CURRENT_LIST_DIR}/logger.hpp
    ${CMAKE_CURRENT_LIST_DIR}/module.hpp
    ${CMAKE_CURRENT_LIST_DIR}/options.hpp
    ${CMAKE_CURRENT_LIST_DIR}/path.hpp
    ${CMAKE_CURRENT_LIST_DIR}/plugin.hpp
    ${CMAKE_CURRENT_LIST_DIR}/plugin-dynlib.hpp
    ${CMAKE_CURRENT_LIST_DIR}/plugin-js.hpp
    ${CMAKE_CURRENT_LIST_DIR}/rule.hpp
    ${CMAKE_CURRENT_LIST_DIR}/server.hpp
    ${CMAKE_CURRENT_LIST_DIR}/server-private.hpp
    ${CMAKE_CURRENT_LIST_DIR}/server-state.hpp
    ${CMAKE_CURRENT_LIST_DIR}/server-state-connected.hpp
    ${CMAKE_CURRENT_LIST_DIR}/server-state-connecting.hpp
    ${CMAKE_CURRENT_LIST_DIR}/server-state-disconnected.hpp
    ${CMAKE_CURRENT_LIST_DIR}/service.hpp
    ${CMAKE_CURRENT_LIST_DIR}/service-command.hpp
    ${CMAKE_CURRENT_LIST_DIR}/service-interrupt.hpp
    ${CMAKE_CURRENT_LIST_DIR}/service-module.hpp
    ${CMAKE_CURRENT_LIST_DIR}/service-plugin.hpp
    ${CMAKE_CURRENT_LIST_DIR}/service-rule.hpp
    ${CMAKE_CURRENT_LIST_DIR}/service-server.hpp
    ${CMAKE_CURRENT_LIST_DIR}/service-transport.hpp
    ${CMAKE_CURRENT_LIST_DIR}/system.hpp
    ${CMAKE_CURRENT_LIST_DIR}/timer.hpp
    ${CMAKE_CURRENT_LIST_DIR}/transport-client.hpp
    ${CMAKE_CURRENT_LIST_DIR}/transport-server.hpp
    ${CMAKE_CURRENT_LIST_DIR}/unicode.hpp
    ${CMAKE_CURRENT_LIST_DIR}/util.hpp
)

set(
    SOURCES
    ${CMAKE_CURRENT_LIST_DIR}/alias.cpp
    ${CMAKE_CURRENT_LIST_DIR}/connection.cpp
    ${CMAKE_CURRENT_LIST_DIR}/config.cpp
    ${CMAKE_CURRENT_LIST_DIR}/cmd-help.cpp
    ${CMAKE_CURRENT_LIST_DIR}/cmd-plugin-config.cpp
    ${CMAKE_CURRENT_LIST_DIR}/cmd-plugin-info.cpp
    ${CMAKE_CURRENT_LIST_DIR}/cmd-plugin-list.cpp
    ${CMAKE_CURRENT_LIST_DIR}/cmd-plugin-load.cpp
    ${CMAKE_CURRENT_LIST_DIR}/cmd-plugin-reload.cpp
    ${CMAKE_CURRENT_LIST_DIR}/cmd-plugin-unload.cpp
    ${CMAKE_CURRENT_LIST_DIR}/cmd-server-cmode.cpp
    ${CMAKE_CURRENT_LIST_DIR}/cmd-server-cnotice.cpp
    ${CMAKE_CURRENT_LIST_DIR}/cmd-server-connect.cpp
    ${CMAKE_CURRENT_LIST_DIR}/cmd-server-disconnect.cpp
    ${CMAKE_CURRENT_LIST_DIR}/cmd-server-info.cpp
    ${CMAKE_CURRENT_LIST_DIR}/cmd-server-invite.cpp
    ${CMAKE_CURRENT_LIST_DIR}/cmd-server-join.cpp
    ${CMAKE_CURRENT_LIST_DIR}/cmd-server-kick.cpp
    ${CMAKE_CURRENT_LIST_DIR}/cmd-server-list.cpp
    ${CMAKE_CURRENT_LIST_DIR}/cmd-server-me.cpp
    ${CMAKE_CURRENT_LIST_DIR}/cmd-server-message.cpp
    ${CMAKE_CURRENT_LIST_DIR}/cmd-server-mode.cpp
    ${CMAKE_CURRENT_LIST_DIR}/cmd-server-nick.cpp
    ${CMAKE_CURRENT_LIST_DIR}/cmd-server-notice.cpp
    ${CMAKE_CURRENT_LIST_DIR}/cmd-server-part.cpp
    ${CMAKE_CURRENT_LIST_DIR}/cmd-server-reconnect.cpp
    ${CMAKE_CURRENT_LIST_DIR}/cmd-server-topic.cpp
    ${CMAKE_CURRENT_LIST_DIR}/cmd-watch.cpp
    ${CMAKE_CURRENT_LIST_DIR}/command.cpp
    ${CMAKE_CURRENT_LIST_DIR}/elapsed-timer.cpp
    ${CMAKE_CURRENT_LIST_DIR}/fs.cpp
    ${CMAKE_CURRENT_LIST_DIR}/ini.cpp
    ${CMAKE_CURRENT_LIST_DIR}/irccd.cpp
    ${CMAKE_CURRENT_LIST_DIR}/irccdctl.cpp
    ${CMAKE_CURRENT_LIST_DIR}/json.cpp
    ${CMAKE_CURRENT_LIST_DIR}/mod-directory.cpp
    ${CMAKE_CURRENT_LIST_DIR}/mod-elapsed-timer.cpp
    ${CMAKE_CURRENT_LIST_DIR}/mod-file.cpp
    ${CMAKE_CURRENT_LIST_DIR}/mod-irccd.cpp
    ${CMAKE_CURRENT_LIST_DIR}/mod-logger.cpp
    ${CMAKE_CURRENT_LIST_DIR}/mod-plugin.cpp
    ${CMAKE_CURRENT_LIST_DIR}/mod-server.cpp
    ${CMAKE_CURRENT_LIST_DIR}/mod-system.cpp
    ${CMAKE_CURRENT_LIST_DIR}/mod-timer.cpp
    ${CMAKE_CURRENT_LIST_DIR}/mod-unicode.cpp
    ${CMAKE_CURRENT_LIST_DIR}/mod-util.cpp
    ${CMAKE_CURRENT_LIST_DIR}/logger.cpp
    ${CMAKE_CURRENT_LIST_DIR}/options.cpp
    ${CMAKE_CURRENT_LIST_DIR}/path.cpp
    ${CMAKE_CURRENT_LIST_DIR}/plugin-dynlib.cpp
    ${CMAKE_CURRENT_LIST_DIR}/plugin-js.cpp
    ${CMAKE_CURRENT_LIST_DIR}/rule.cpp
    ${CMAKE_CURRENT_LIST_DIR}/server.cpp
    ${CMAKE_CURRENT_LIST_DIR}/server-state-connected.cpp
    ${CMAKE_CURRENT_LIST_DIR}/server-state-connecting.cpp
    ${CMAKE_CURRENT_LIST_DIR}/server-state-disconnected.cpp
    ${CMAKE_CURRENT_LIST_DIR}/service-command.cpp
    ${CMAKE_CURRENT_LIST_DIR}/service-interrupt.cpp
    ${CMAKE_CURRENT_LIST_DIR}/service-module.cpp
    ${CMAKE_CURRENT_LIST_DIR}/service-plugin.cpp
    ${CMAKE_CURRENT_LIST_DIR}/service-rule.cpp
    ${CMAKE_CURRENT_LIST_DIR}/service-server.cpp
    ${CMAKE_CURRENT_LIST_DIR}/service-transport.cpp
    ${CMAKE_CURRENT_LIST_DIR}/system.cpp
    ${CMAKE_CURRENT_LIST_DIR}/timer.cpp
    ${CMAKE_CURRENT_LIST_DIR}/transport-client.cpp
    ${CMAKE_CURRENT_LIST_DIR}/transport-server.cpp
    ${CMAKE_CURRENT_LIST_DIR}/unicode.cpp
    ${CMAKE_CURRENT_LIST_DIR}/util.cpp
)

if (NOT IRCCD_SYSTEM_WINDOWS)
    list(APPEND HEADERS ${CMAKE_CURRENT_LIST_DIR}/xdg.hpp)
endif ()
