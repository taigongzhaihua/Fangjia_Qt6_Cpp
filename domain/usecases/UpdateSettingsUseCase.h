#pragma once
#include "entities/Settings.h"
#include "repositories/ISettingsRepository.h"
#include <memory>

namespace domain::usecases
{

	/// Use case for updating application settings
/// Pure C++ - no Qt dependencies
	class UpdateSettingsUseCase {
	public:
		explicit UpdateSettingsUseCase(std::shared_ptr<repositories::ISettingsRepository> repository);
		~UpdateSettingsUseCase() = default;

		/// Execute the use case - update settings and optionally save immediately
		void execute(const entities::Settings& settings, bool saveImmediately = true);

	private:
		std::shared_ptr<repositories::ISettingsRepository> m_repository;
	};

}
