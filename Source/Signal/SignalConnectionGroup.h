#ifndef _SIGNAL_CONNECTION_GROUP_H_
#define _SIGNAL_CONNECTION_GROUP_H_

#include <boost/signals2.hpp>

#include <vector>

class SignalConnectionGroup final {
public:
	SignalConnectionGroup();
	template <typename ...Arguments, typename = typename std::enable_if<sizeof...(Arguments) >= 1>::type>
	SignalConnectionGroup(Arguments &&... arguments);
	SignalConnectionGroup(SignalConnectionGroup && g) noexcept;
	SignalConnectionGroup(const SignalConnectionGroup & g);
	SignalConnectionGroup & operator = (SignalConnectionGroup && g) noexcept;
	SignalConnectionGroup & operator = (const SignalConnectionGroup & g);
	~SignalConnectionGroup();

	void addConnection(const boost::signals2::connection & connection);
	void clear();
	void disconnect();

private:
	std::vector<boost::signals2::connection> m_connections;
};

template <typename ...Arguments, typename>
SignalConnectionGroup::SignalConnectionGroup(Arguments &&... arguments) {
	boost::signals2::connection unpackedArguments[sizeof...(arguments)] = {arguments...};

	for(size_t i = 0; i < sizeof...(arguments); i++) {
		m_connections.push_back(unpackedArguments[i]);
	}
}

#endif // _SIGNAL_CONNECTION_GROUP_H_
