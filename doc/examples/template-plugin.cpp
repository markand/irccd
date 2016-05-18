/*
 * This example file show how to fill irccd callbacks for a native plugin.
 *
 * All of the defined callbacks are optional and may be removed.
 */

#include <iostream>

#include <irccd/plugin-dynlib.hpp>	// (in irccd_onReload, irccd_onLoad and irccd_onUnload)
#include <irccd/server.hpp>
#include <irccd/util.hpp>		// for util::join (in irccd_onNames)

using namespace irccd;

extern "C" {

/* --- onCommand ---------------------------------------------------- */

void irccd_onCommand(Irccd &,
		     const std::shared_ptr<Server> &server,
		     const std::string &origin,
		     const std::string &channel,
		     const std::string &message)
{
	std::cout << "onCommand: server=" << server->name()
		  << ", origin=" << origin
		  << ", channel=" << channel
		  << ", message=" << message << std::endl;
}

/* --- onConnect ---------------------------------------------------- */

void irccd_onConnect(Irccd &, const std::shared_ptr<Server> &server)
{
	std::cout << "onConnect: server=" << server->name() << std::endl;
}

/* --- onChannelMode ------------------------------------------------ */

void irccd_onChannelMode(Irccd &,
			 const std::shared_ptr<Server> &server,
			 const std::string &origin,
			 const std::string &channel,
			 const std::string &mode,
			 const std::string &arg)
{
	std::cout << "onChannelMode: server=" << server->name()
		  << ", origin=" << origin
		  << ", channel=" << channel
		  << ", mode=" << mode
		  << ", arg=" << arg << std::endl;
}

/* --- onChannelNotice ---------------------------------------------- */

void irccd_onChannelNotice(Irccd &irccd,
			   const std::shared_ptr<Server> &server,
			   const std::string &origin,
			   const std::string &channel,
			   const std::string &notice)
{
	std::cout << "onChannelNotice: server=" << server->name()
		  << ", origin=" << origin
		  << ", channel=" << channel
		  << ", notice=" << notice << std::endl;
}

/* --- onInvite ----------------------------------------------------- */

void irccd_onInvite(Irccd &irccd, const std::shared_ptr<Server> &server, const std::string &origin, const std::string &channel)
{
	std::cout << "onInvite: server=" << server->name() << ", origin=" << origin << ", channel=" << channel << std::endl;
}

/* --- onJoin ------------------------------------------------------- */

void irccd_onJoin(Irccd &irccd, const std::shared_ptr<Server> &server, const std::string &origin, const std::string &channel)
{
	std::cout << "onJoin: server=" << server->name() << ", origin=" << origin << ", channel=" << channel << std::endl;
}

/* --- onKick ------------------------------------------------------- */

void irccd_onKick(Irccd &irccd,
		  const std::shared_ptr<Server> &server,
		  const std::string &origin,
		  const std::string &channel,
		  const std::string &target,
		  const std::string &reason)
{
	std::cout << "onKick: server=" << server->name()
		  << ", origin=" << origin
		  << ", channel=" << channel
		  << ", target=" << channel
		  << ", reason=" << reason << std::endl;
}

/* --- onLoad ------------------------------------------------------- */

void irccd_onLoad(Irccd &, DynlibPlugin &plugin)
{
	std::cout << "onLoad: plugin=" << plugin.name() << std::endl;
}

/* --- onMessage ---------------------------------------------------- */

void irccd_onMessage(Irccd &irccd,
		     const std::shared_ptr<Server> &server,
		     const std::string &origin,
		     const std::string &channel,
		     const std::string &message)
{
	std::cout << "onMessage: server=" << server->name()
		  << ", origin=" << origin
		  << ", channel=" << channel
		  << ", message=" << message << std::endl;
}

/* --- onMe --------------------------------------------------------- */

void irccd_onMe(Irccd &irccd,
		const std::shared_ptr<Server> &server,
		const std::string &origin,
		const std::string &channel,
		const std::string &message)
{
	std::cout << "onMe: server=" << server->name()
		  << ", origin=" << origin
		  << ", channel=" << channel
		  << ", message=" << message << std::endl;
}

/* --- onMode ------------------------------------------------------- */

void irccd_onMode(Irccd &irccd, const std::shared_ptr<Server> &server, const std::string &origin, const std::string &mode)
{
	std::cout << "onMode: server=" << server->name() << ", origin=" << origin << ", mode=" << mode << std::endl;
}

/* --- onNames ------------------------------------------------------ */

void irccd_onNames(Irccd &irccd,
		   const std::shared_ptr<Server> &server,
		   const std::string &channel,
		   const std::vector<std::string> &list)
{
	std::cout << "onNames: server=" << server->name()
		  << ", channel=" << channel
		  << ", list=" << util::join(list.begin(), list.end(), ", ") << std::endl;
}

/* --- onNick ------------------------------------------------------- */

void irccd_onNick(Irccd &irccd, const std::shared_ptr<Server> &server, const std::string &origin, const std::string &nick)
{
	std::cout << "onNick: server=" << server->name() << ", origin=" << origin << ", nick=" << nick << std::endl;
}

/* --- onNotice ----------------------------------------------------- */

void irccd_onNotice(Irccd &irccd, const std::shared_ptr<Server> &server, const std::string &origin, const std::string &notice)
{
	std::cout << "onNotice: server=" << server->name() << ", origin=" << origin << ", notice=" << notice << std::endl;
}

/* --- onPart ------------------------------------------------------- */

void irccd_onPart(Irccd &irccd,
		  const std::shared_ptr<Server> &server,
		  const std::string &origin,
		  const std::string &channel,
		  const std::string &reason)
{
	std::cout << "onPart: server=" << server->name()
		  << ", origin=" << origin
		  << ", channel=" << channel
		  << ", reason=" << reason << std::endl;
}

/* --- onQuery ------------------------------------------------------ */

void irccd_onQuery(Irccd &, const std::shared_ptr<Server> &server, const std::string &origin, const std::string &message)
{
	std::cout << "onQuery: server=" << server->name() << ", origin=" << origin << ", message=" << message << std::endl;
}

/* --- onQueryCommand ----------------------------------------------- */

void irccd_onQueryCommand(Irccd &, const std::shared_ptr<Server> &server, const std::string &origin, const std::string &message)
{
	std::cout << "onQueryCommand: server=" << server->name() << ", origin=" << origin << ", message=" << message << std::endl;
}

/* --- onReload ----------------------------------------------------- */

void irccd_onReload(Irccd &irccd, DynlibPlugin &plugin)
{
	std::cout << "onReload: plugin=" << plugin.name() << std::endl;
}

/* --- onTopic ------------------------------------------------------ */

void irccd_onTopic(Irccd &irccd,
		   const std::shared_ptr<Server> &server,
		   const std::string &origin,
		   const std::string &channel,
		   const std::string &topic)
{
	std::cout << "onTopic: server=" << server->name()
		  << ", origin=" << origin
		  << ", channel=" << channel
		  << ", topic=" << topic << std::endl;
}

/* --- onUnload ----------------------------------------------------- */

void irccd_onUnload(Irccd &irccd, DynlibPlugin &plugin)
{
	std::cout << "onUnload: plugin=" << plugin.name() << std::endl;
}

/* --- onWhois ------------------------------------------------------ */

void irccd_onWhois(Irccd &irccd, const std::shared_ptr<Server> &server, const ServerWhois &info)
{
	std::cout << "onWhois: server=" << server->name() << ", info-for=" << info.nick << std::endl;
}

} // !C
