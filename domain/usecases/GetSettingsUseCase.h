#pragma once
#include "entities/Settings.h"
#include "repositories/ISettingsRepository.h"
#include <memory>

namespace domain::usecases
{

	/// Use case for retrieving application settings
/// Pure C++ - no Qt dependencies
	class GetSettingsUseCase {
	public:
		explicit GetSettingsUseCase(std::shared_ptr<repositories::ISettingsRepository> repository);
		~GetSettingsUseCase() = default;

		/// Execute the use case - get current settings
		entities::Settings execute() const;

	private:
		std::shared_ptr<repositories::ISettingsRepository> m_repository;
	};

}
