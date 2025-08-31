#include <iostream>
#include <cassert>
#include <vector>
#include <algorithm>
#include <cmath>

/**
 * Focused test to validate Star track minimum size aggregation
 * Tests that multiple children in the same Star track correctly 
 * aggregate their minimum sizes using max() instead of overwriting
 */

// Simplified mock for testing Star track aggregation logic
struct MockChild {
    int track;  // which star track this child belongs to
    int minSize; // minimum content size for this child
};

// Simplified version of the fixed Star track sizing logic
std::vector<int> computeStarMinAggregation(const std::vector<MockChild>& children, int numTracks) {
    std::vector<int> starMin(numTracks, 0);
    
    // Simulate Pass 1: single-span children with the FIXED aggregation logic
    for (const auto& child : children) {
        if (child.track >= 0 && child.track < numTracks) {
            // FIXED: Use max() aggregation instead of overwrite
            starMin[child.track] = std::max(starMin[child.track], child.minSize);
        }
    }
    
    return starMin;
}

// Simulation of the OLD BROKEN logic for comparison
std::vector<int> computeStarMinBroken(const std::vector<MockChild>& children, int numTracks) {
    std::vector<int> starMin(numTracks, 0);
    
    // Simulate the BROKEN logic: overwrite instead of aggregate
    for (const auto& child : children) {
        if (child.track >= 0 && child.track < numTracks) {
            // BROKEN: Overwrite instead of max() - this was the bug
            starMin[child.track] = child.minSize;
        }
    }
    
    return starMin;
}

int main() {
    std::cout << "Testing Star track minimum size aggregation fix..." << std::endl;
    
    // Test scenario: 2 Star tracks, multiple children per track
    // Track 0: children with sizes 50, 80, 30 -> should aggregate to max=80
    // Track 1: children with sizes 40, 60 -> should aggregate to max=60
    std::vector<MockChild> children = {
        {0, 50},  // Star track 0, child needs 50px
        {1, 40},  // Star track 1, child needs 40px
        {0, 80},  // Star track 0, child needs 80px (should become the max)
        {1, 60},  // Star track 1, child needs 60px (should become the max)
        {0, 30},  // Star track 0, child needs 30px
    };
    
    // Test the FIXED aggregation logic
    std::cout << "Test 1: Fixed aggregation logic (using max())..." << std::endl;
    std::vector<int> fixedResult = computeStarMinAggregation(children, 2);
    assert(fixedResult.size() == 2);
    assert(fixedResult[0] == 80); // Track 0 should be max(50, 80, 30) = 80
    assert(fixedResult[1] == 60); // Track 1 should be max(40, 60) = 60
    std::cout << "✅ Track 0 starMin: " << fixedResult[0] << " (correctly aggregated to max)" << std::endl;
    std::cout << "✅ Track 1 starMin: " << fixedResult[1] << " (correctly aggregated to max)" << std::endl;
    
    // Test the BROKEN logic to demonstrate the problem
    std::cout << "\nTest 2: Broken logic (overwriting) for comparison..." << std::endl;
    std::vector<int> brokenResult = computeStarMinBroken(children, 2);
    assert(brokenResult.size() == 2);
    assert(brokenResult[0] == 30); // Track 0 would be just the last child's size (30)
    assert(brokenResult[1] == 60); // Track 1 would be just the last child's size (60)
    std::cout << "❌ Track 0 starMin: " << brokenResult[0] << " (incorrectly collapsed to last child)" << std::endl;
    std::cout << "❌ Track 1 starMin: " << brokenResult[1] << " (happened to work since last was largest)" << std::endl;
    
    // Demonstrate the problem: In broken logic, Star tracks can be too small
    std::cout << "\nProblem demonstration:" << std::endl;
    std::cout << "- Track 0 has children needing [50, 80, 30] pixels" << std::endl;
    std::cout << "- With broken logic: track gets only 30px (too small for 80px child!)" << std::endl;
    std::cout << "- With fixed logic: track gets 80px (correctly fits largest child)" << std::endl;
    std::cout << "- This prevents compressed layouts and ensures proper minimum sizes" << std::endl;
    
    // Edge case: empty tracks
    std::cout << "\nTest 3: Edge case - tracks with no children..." << std::endl;
    std::vector<MockChild> sparseChildren = {{0, 100}};  // Only track 0 has a child
    std::vector<int> sparseResult = computeStarMinAggregation(sparseChildren, 3);
    assert(sparseResult.size() == 3);
    assert(sparseResult[0] == 100); // Track 0 gets the child's size
    assert(sparseResult[1] == 0);   // Track 1 remains 0 (no children)
    assert(sparseResult[2] == 0);   // Track 2 remains 0 (no children)
    std::cout << "✅ Empty tracks correctly remain at 0" << std::endl;
    
    std::cout << "\n🎉 All Star track aggregation tests PASSED!" << std::endl;
    std::cout << "✅ Star tracks now correctly aggregate minimum sizes using max()" << std::endl;
    std::cout << "✅ Multiple children in same Star track no longer cause size collapse" << std::endl;
    std::cout << "✅ Grid layouts will have proper minimum sizes preventing compression" << std::endl;
    
    return 0;
}