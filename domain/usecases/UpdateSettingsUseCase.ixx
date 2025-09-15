// UpdateSettingsUseCase.ixx - C++20 module interface for UpdateSettingsUseCase
module;

#include <memory>

export module fangjia.domain.usecases.update_settings;

import fangjia.domain.entities.settings;

// Forward declarations for internal dependencies
export namespace domain::repositories {
	class ISettingsRepository;
}

export namespace domain::usecases
{
	/// Use case for updating application settings
	/// Pure C++ - no Qt dependencies
	class UpdateSettingsUseCase {
	public:
		explicit UpdateSettingsUseCase(std::shared_ptr<repositories::ISettingsRepository> repository);
		~UpdateSettingsUseCase() = default;

		/// Execute the use case - update settings and optionally save immediately
		void execute(const entities::Settings& settings, bool saveImmediately = true) const;

	private:
		std::shared_ptr<repositories::ISettingsRepository> m_repository;
	};
}