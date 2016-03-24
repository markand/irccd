#ifndef _REMOTE_COMMAND_H_
#define _REMOTE_COMMAND_H_

#include <cassert>
#include <cstdint>
#include <map>
#include <vector>

#include <irccd/json.h>

namespace irccd {

class Irccd;
class Irccdctl;

/**
 * @class RemoteCommandOption
 * @brief Describe a command line option
 */
class RemoteCommandOption {
public:
	enum {
		Argument = (1 << 0)	//!< option requires an argument
	};

private:
	std::string m_id;
	std::string m_simple;
	std::string m_long;
	std::string m_description;
	std::uint8_t m_flags;

public:
	/**
	 * Constructor an option description.
	 *
	 * @pre id must not be empty
	 * @pre at least simpleKey or longKey must not be empty
	 * @pre description must not be empty
	 * @param key the key the option key
	 * @param description the description
	 * @param flags the optional flags
	 */
	inline RemoteCommandOption(std::string id,
				   std::string simpleKey,
				   std::string longKey,
				   std::string description,
				   std::uint8_t flags = 0) noexcept
		: m_id(std::move(id))
		, m_simple(std::move(simpleKey))
		, m_long(std::move(longKey))
		, m_description(std::move(description))
		, m_flags(flags)
	{
		assert(!m_id.empty());
		assert(!m_simple.empty() || !m_long.empty());
		assert(!m_description.empty());
	}

	/**
	 * Get the id.
	 *
	 * @return the id
	 */
	inline const std::string &id() const noexcept
	{
		return m_id;
	}

	/**
	 * Get the option key.
	 *
	 * @return the key
	 */
	inline const std::string &simpleKey() const noexcept
	{
		return m_simple;
	}

	/**
	 * Get the long option.
	 *
	 * @return the long option
	 */
	inline const std::string &longKey() const noexcept
	{
		return m_long;
	}

	/**
	 * Get the option description.
	 *
	 * @return the description
	 */
	inline const std::string &description() const noexcept
	{
		return m_description;
	}

	/**
	 * Get the option flags.
	 *
	 * @return the flags
	 */
	inline std::uint8_t flags() const noexcept
	{
		return m_flags;
	}
};

/**
 * @brief List of command line options.
 */
using RemoteCommandOptions = std::vector<RemoteCommandOption>;

using RemoteCommandArg = std::pair<std::string, bool>;

/**
 * @brief List of arguments to pass to the command.
 *
 * Any argument must have a non-empty name an can be optional if the boolean is set to false.
 */
using RemoteCommandArgs = std::vector<std::pair<std::string, bool>>;

class RemoteCommandRequest {
private:
	std::multimap<std::string, std::string> m_options;
	std::vector<std::string> m_args;

public:
	inline RemoteCommandRequest(std::multimap<std::string, std::string> options, std::vector<std::string> args) noexcept
		: m_options(std::move(options))
		, m_args(std::move(args))
	{
	}

	inline const std::vector<std::string> &args() const noexcept
	{
		return m_args;
	}

	inline const std::multimap<std::string, std::string> &options() const noexcept
	{
		return m_options;
	}

	inline unsigned length() const noexcept
	{
		return m_args.size();
	}

	inline bool has(const std::string &option) const noexcept
	{
		return m_options.count(option) != 0;
	}

	const std::string &arg(unsigned index) const noexcept;

	std::string argOr(unsigned index, std::string defaultValue) const noexcept;

	const std::string &option(const std::string &key) const noexcept;

	std::string optionOr(const std::string &key, std::string defaultValue) const noexcept;
};

class RemoteCommand {
private:
	std::string m_name;
	std::string m_category;
	bool m_visible;

public:
	inline RemoteCommand(std::string name, std::string category, bool visible = true) noexcept
		: m_name(std::move(name))
		, m_category(std::move(category))
		, m_visible(visible)
	{
		assert(!m_name.empty());
		assert(!m_category.empty());
	}

	/**
	 * Default destructor virtual.
	 */
	virtual ~RemoteCommand() = default;

	/**
	 * Return the command name, must not have spaces.
	 *
	 * @return the command name
	 */
	inline const std::string &name() const noexcept
	{
		return m_name;
	}

	/**
	 * Get the command category.
	 *
	 * Irccdctl will sort commands by categories.
	 *
	 * @return the category
	 */
	inline const std::string &category() const noexcept
	{
		return m_category;
	}

	/**
	 * Hide the command in non-verbose mode.
	 *
	 * @return true if the command should be visible in non-verbose mode
	 */
	inline bool visible() const noexcept
	{
		return m_visible;
	}

	/**
	 * Return the command usage, without the prefix. (e.g. host port).
	 *
	 * Options are prepended automatically
	 *
	 * @return the usage
	 */
	std::string usage() const;

	/**
	 * Return the help message for irccdctl invocation.
	 *
	 * @return the help
	 */
	virtual std::string help() const = 0;

	/**
	 * Get the supported irccdctl options.
	 *
	 * @return the options
	 */
	virtual RemoteCommandOptions options() const
	{
		return RemoteCommandOptions();
	}

	/**
	 * Get the supported arguments.
	 *
	 * @return the arguments
	 */
	virtual RemoteCommandArgs args() const
	{
		return RemoteCommandArgs();
	}

	/**
	 * Prepare a JSON request to the daemon.
	 *
	 * If the command is local and does not need to send anything to irccd's instance, return a null JSON value.
	 *
	 * The default implementation just send the command name with no arguments.
	 *
	 * @param irccdctl the irccdctl instance
	 * @param args the command line arguments and options
	 * @return the JSON object to send to the daemon
	 * @post the returned JSON value must be an object
	 */
	virtual json::Value request(Irccdctl &irccdctl, const RemoteCommandRequest &args) const;

	/**
	 * Execute the command in the daemon.
	 *
	 * The user can return an object with any properties to forward to the client. Irccd will automatically
	 * add the command name and the appropriate status code.
	 *
	 * The default return an empty object which indicates success.
	 *
	 * If any exception is thrown from this function, it is forwarded to the client as error status.
	 *
	 * @param irccd the instance
	 * @param request the JSON request
	 * @return the response
	 */
	virtual json::Value exec(Irccd &irccd, const json::Value &request) const;

	/**
	 * What to do when receiving the response from irccd.
	 *
	 * This default implementation just check for an error string and shows it if any.
	 * 
	 * @param irccdctl the irccdctl instane
	 * @param object the result
	 */
	virtual void result(Irccdctl &irccdctl, const json::Value &response) const;
};

} // !irccd

#endif // !_REMOTE_COMMAND_H_
