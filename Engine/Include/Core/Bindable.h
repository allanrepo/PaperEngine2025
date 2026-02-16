#pragma once

namespace core
{
	class IBindable
	{
	public:
		virtual ~IBindable() = default;
		virtual void Bind() const = 0;
		virtual bool CanBind() const = 0;
	};
}