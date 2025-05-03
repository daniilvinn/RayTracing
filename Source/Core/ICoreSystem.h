#pragma once

#include "Common.h"

namespace RT {

	class ICoreSystem {
	public:
		virtual void Initialize(void* InputData) = 0;
		virtual void Shutdown() = 0;

		bool Initialized() const {
			return IsInitialized;
		}

		std::string_view GetFailureReason() const { 
			return FailureReason; 
		}

	protected:
		bool IsInitialized;
		std::string FailureReason;

	};

}