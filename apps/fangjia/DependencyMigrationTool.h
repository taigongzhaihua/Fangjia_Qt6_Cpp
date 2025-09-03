#pragma once
#include <map>
#include <string>
#include <vector>

/// Migration tool for dependency injection unification
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
	};

	static DependencyMigrationTool& instance();

	/// Migrate a specific service from legacy to Boost.DI
	/// @param serviceName The name of the service to migrate
	/// @return true if migration was successful or already completed
	bool migrateService(const std::string& serviceName);

	/// Validate the current migration state
	/// @return true if all migrated services are working correctly
	bool validateMigration();

	/// Generate a comprehensive migration report
	/// @return Report containing migration status of all services
	MigrationReport generateMigrationReport();

	/// Add a service to the migration tracking
	void trackService(const std::string& serviceName, const std::string& currentSystem, const std::string& targetSystem);

	/// Mark a service migration as completed
	void markServiceMigrated(const std::string& serviceName);

	/// Get migration status for a specific service
	MigrationStatus getServiceStatus(const std::string& serviceName) const;

	/// Check if all services have been migrated
	bool isFullyMigrated() const;

	/// Get list of services pending migration
	std::vector<std::string> getPendingServices() const;

private:
	DependencyMigrationTool();

	std::map<std::string, ServiceMigrationInfo> m_serviceInfo;

	void initializeKnownServices();
	bool validateService(const std::string& serviceName);
};