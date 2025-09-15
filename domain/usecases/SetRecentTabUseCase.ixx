// SetRecentTabUseCase.ixx - C++20 module interface for SetRecentTabUseCase
module;

#include <memory>
#include <string>

export module fangjia.domain.usecases.set_recent_tab;

// Forward declarations for internal dependencies
export namespace domain::repositories {
	class ISettingsRepository;
}

export namespace domain::usecases
{
	/// Use case for setting the recent tab setting
	class SetRecentTabUseCase {
	public:
		explicit SetRecentTabUseCase(std::shared_ptr<repositories::ISettingsRepository> repository);

		/// Set the recent tab ID in settings and save
		/// @param tabId New recent tab ID to set
		void execute(const std::string& tabId) const;

	private:
		std::shared_ptr<repositories::ISettingsRepository> m_repository;
	};
}