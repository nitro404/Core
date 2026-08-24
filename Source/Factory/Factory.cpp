#include "Factory.h"

Factory::Factory() { }

Factory::Factory(Factory && factory) noexcept { }

Factory & Factory::operator = (Factory factory) noexcept {
	return *this;
}

Factory::~Factory() { }
