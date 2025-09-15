// GetSettingsUseCase.ixx - C++20 module interface for GetSettingsUseCase
module;

#include <memory>

export module fangjia.domain.usecases.get_settings;

import fangjia.domain.entities.settings;

// Forward declarations for internal dependencies
export namespace domain::repositories {
	class ISettingsRepository;
}

export namespace domain::usecases
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