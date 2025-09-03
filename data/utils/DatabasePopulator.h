/*
 * DatabasePopulator.h
 *
 * Utility to populate the database with sample data for testing
 * the Formula functionality with real database schema.
 */

#pragma once

class QSqlDatabase;

namespace data::utils {

	/// Utility class to populate database with sample formula data
	class DatabasePopulator {
	public:
		/// Populate the database with sample categories and formulations
		/// Parameters: db - Database connection to populate
		/// Returns: true if successful, false otherwise
		static bool populateSampleData(QSqlDatabase& db);

	private:
		/// Create sample categories
		static bool createSampleCategories(QSqlDatabase& db);

		/// Create sample formulations
		static bool createSampleFormulations(QSqlDatabase& db);

		/// Create sample formulation compositions
		static bool createSampleCompositions(QSqlDatabase& db);
	};

} // namespace data::utils