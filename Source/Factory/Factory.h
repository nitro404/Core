#ifndef _FACTORY_H_
#define _FACTORY_H_

#include <functional>
#include <memory>

class Factory {
public:
	Factory();
	virtual ~Factory();

	template <class T>
	std::function<std::unique_ptr<T>()> getFunction() const;

	template <class T>
	std::unique_ptr<T> callFunction() const;

private:
	Factory(Factory && f) noexcept = delete;
	Factory(const Factory & f) = delete;
	Factory & operator = (Factory && f) noexcept = delete;
	Factory & operator = (const Factory & f) = delete;
};

template <class T>
class FactoryFunction final : public Factory {
public:
	FactoryFunction(const std::function<std::unique_ptr<T>()> & func);
	virtual ~FactoryFunction();

	std::unique_ptr<T> callFunctionDirect() const;
	const std::function<std::unique_ptr<T>()> & getFunctionDirect() const;

private:
	std::function<std::unique_ptr<T>()> m_function;

	FactoryFunction(FactoryFunction<T> && f) noexcept = delete;
	FactoryFunction(const FactoryFunction<T> & f = delete);
	FactoryFunction<T> & operator = (FactoryFunction<T> && f) noexcept = delete;
	FactoryFunction<T> & operator = (const FactoryFunction<T> & f) = delete;
};

template <class T>
FactoryFunction<T>::FactoryFunction(const std::function<std::unique_ptr<T>()> & func)
	: Factory()
	, m_function(func) { }

template <class T>
FactoryFunction<T>::~FactoryFunction() { }

template <class T>
std::function<std::unique_ptr<T>()> Factory::getFunction() const {
	const FactoryFunction<T> * factoryFunction(dynamic_cast<const FactoryFunction<T> *>(this));

	if(factoryFunction == nullptr) {
		return nullptr;
	}

	return factoryFunction->getFunctionDirect();
}

template <class T>
std::unique_ptr<T> Factory::callFunction() const {
	const FactoryFunction<T> * factoryFunction(dynamic_cast<const FactoryFunction<T> *>(this));

	if(factoryFunction == nullptr) {
		return nullptr;
	}

	return factoryFunction->getFunctionDirect()();
}

template <class T>
const std::function<std::unique_ptr<T>()> & FactoryFunction<T>::getFunctionDirect() const {
	return m_function;
}

template <class T>
std::unique_ptr<T> FactoryFunction<T>::callFunctionDirect() const {
	return m_function();
}

#endif // _FACTORY_H_
