#include <sstream>

#include <irccd/logger.h>
#include <irccd/system.h>

#include "command.h"

namespace irccd {

/*
 * RemoteCommandRequest
 * ------------------------------------------------------------------
 */

const std::string &RemoteCommandRequest::arg(unsigned index) const noexcept
{
	assert(index < m_args.size());

	return m_args[index];
}

std::string RemoteCommandRequest::argOr(unsigned index, std::string defaultValue) const noexcept
{
	return index < m_args.size() ? m_args[index] : defaultValue;
}

const std::string &RemoteCommandRequest::option(const std::string &key) const noexcept
{
	assert(m_options.count(key) != 0);

	return m_options.find(key)->second;
}

std::string RemoteCommandRequest::optionOr(const std::string &key, std::string defaultValue) const noexcept
{
	auto it = m_options.find(key);

	if (it == m_options.end())
		return defaultValue;

	return it->second;
}

/*
 * RemoteCommand
 * ------------------------------------------------------------------
 */

std::string RemoteCommand::usage() const
{
	std::ostringstream oss;

	oss << "usage: sys::programName()";

	/* Options summary */
	if (options().size() > 0)
		oss << " [options...]";

	/* Arguments */
	for (const RemoteCommandArg &arg : args()) {
		if (!arg.second)
			oss << "[";

		oss << arg.first;

		if (!arg.second)
			oss << "]";
	}

	return oss.str();
}

json::Value RemoteCommand::request(Irccdctl &, const RemoteCommandRequest &) const
{
	return nullptr;
}

json::Value RemoteCommand::exec(Irccd &, const json::Value &) const
{
	return json::object({});
}

void RemoteCommand::result(Irccdctl &, const json::Value &response) const
{
	auto it = response.find("error");

	if (it != response.end() && it->isString())
		log::warning() << "irccdctl: " << it->toString() << std::endl;
}

} // !irccd
