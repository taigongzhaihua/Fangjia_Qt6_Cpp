// GetRecentTabUseCase.ixx - C++20 module interface for GetRecentTabUseCase
module;

#include <memory>
#include <string>

export module fangjia.domain.usecases.get_recent_tab;

// Forward declarations for internal dependencies
export namespace domain::repositories {
	class ISettingsRepository;
}

export namespace domain::usecases
{
	/// Use case for retrieving the recent tab setting
	class GetRecentTabUseCase {
	public:
		explicit GetRecentTabUseCase(std::shared_ptr<repositories::ISettingsRepository> repository);

		/// Get the recent tab ID from settings
		/// @return Recent tab ID as string (empty if not set)
		std::string execute() const;

	private:
		std::shared_ptr<repositories::ISettingsRepository> m_repository;
	};
}