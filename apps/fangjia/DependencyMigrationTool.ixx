// DependencyMigrationTool.ixx - C++20 module interface for DependencyMigrationTool
module;

#include <map>
#include <string>
#include <vector>

export module fangjia.apps.dependency_migration_tool;

export /// Migration tool for dependency injection unification
/// Implements the migration tool concept from architecture-analysis.md Phase 3
class DependencyMigrationTool {
public:
	enum class MigrationStatus {
		NotStarted,
		InProgress,
		Completed,
		Failed
	};

	struct ServiceMigrationInfo {
		std::string serviceName;
		std::string currentSystem;  // "Boost.DI" or "Legacy"
		std::string targetSystem;   // "Boost.DI"
		MigrationStatus status;
		std::string notes;
	};

	struct MigrationReport {
		std::vector<ServiceMigrationInfo> services;
		int totalServices = 0;
		int migratedServices = 0;
		int pendingServices = 0;
		double completionPercentage = 0.0;
		std::string summary;
	};

	static DependencyMigrationTool& instance();

	/// Generate a comprehensive migration report
	MigrationReport generateMigrationReport() const;

	/// Validate that a service has been properly migrated
	bool validateServiceMigration(const std::string& serviceName) const;

	/// Get the status of a specific service migration
	MigrationStatus getServiceMigrationStatus(const std::string& serviceName) const;

	/// Mark a service as migrated to Boost.DI
	void markServiceMigrated(const std::string& serviceName, const std::string& notes = "");

	/// Mark a service migration as failed
	void markServiceMigrationFailed(const std::string& serviceName, const std::string& reason);

	/// Get a list of all services that need migration
	std::vector<std::string> getPendingMigrations() const;

	/// Reset all migration status (for testing)
	void resetMigrationStatus();

private:
	DependencyMigrationTool();
	void initializeServiceList();

	std::map<std::string, ServiceMigrationInfo> m_services;
};