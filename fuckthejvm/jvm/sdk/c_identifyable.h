#pragma once

class c_identifyable {
protected:
	template <typename T>
	T get_value(std::uint32_t offset) const
	{
		return *reinterpret_cast<T*>((std::uintptr_t)this + offset);
	}

	template <typename T>
	T get_value_direct(std::uint32_t address) const
	{
		return *reinterpret_cast<T*>(address);
	}
};