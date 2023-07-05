#include "SignalConnectionGroup.h"

SignalConnectionGroup::SignalConnectionGroup() { }

SignalConnectionGroup::SignalConnectionGroup(SignalConnectionGroup && g) noexcept
	: m_connections(std::move(g.m_connections)) { }

SignalConnectionGroup::SignalConnectionGroup(const SignalConnectionGroup & g)
	: m_connections(g.m_connections) { }

SignalConnectionGroup & SignalConnectionGroup::operator = (SignalConnectionGroup && g) noexcept {
	if(this != &g) {
		m_connections = std::move(g.m_connections);
	}

	return *this;
}

SignalConnectionGroup & SignalConnectionGroup::operator = (const SignalConnectionGroup & g) {
	m_connections = g.m_connections;

	return *this;
}

SignalConnectionGroup::~SignalConnectionGroup() { }

void SignalConnectionGroup::addConnection(const boost::signals2::connection & connection) {
	m_connections.push_back(connection);
}

void SignalConnectionGroup::clear() {
	m_connections.clear();
}

void SignalConnectionGroup::disconnect() {
	for(boost::signals2::connection & connection : m_connections) {
		connection.disconnect();
	}

	clear();
}
