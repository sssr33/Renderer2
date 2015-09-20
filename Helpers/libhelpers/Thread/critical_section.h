#pragma once
#include "..\Macros.h"

#include <Windows.h>

namespace thread {
	class condition_variable;

	class critical_section {
	public:
		NO_COPY_MOVE(critical_section);

		// default value of spinCount - https://msdn.microsoft.com/en-us/library/windows/desktop/ms683477%28v=vs.85%29.aspx
		critical_section(DWORD spinCount = 4000) {
			InitializeCriticalSectionEx(&this->cs, spinCount, 0);
		}

		~critical_section() {
			DeleteCriticalSection(&this->cs);
		}
	private:
		CRITICAL_SECTION cs;

		friend class condition_variable;

	public:
		class scoped_lock {
		public:
			scoped_lock()
				: cs(nullptr) {
			}

			scoped_lock(critical_section &cs)
				: cs(&cs) {
				EnterCriticalSection(&this->cs->cs);
			}

			scoped_lock(const scoped_lock &other) = delete;

			scoped_lock(scoped_lock &&other)
				: cs(other.cs)
			{
				other.cs = nullptr;
			}

			~scoped_lock() {
				if (this->cs) {
					LeaveCriticalSection(&this->cs->cs);
				}
			}

			scoped_lock &operator=(const scoped_lock &other) = delete;

			scoped_lock &operator=(scoped_lock &&other) {
				if (this != &other) {
					if (this->cs) {
						LeaveCriticalSection(&this->cs->cs);
					}

					this->cs = other.cs;
					other.cs = nullptr;
				}

				return *this;
			}
		private:
			critical_section *cs;
		};
	};
}