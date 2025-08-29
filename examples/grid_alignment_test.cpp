/*
 * Example demonstrating UiGrid alignment fix
 * 
 * This example shows how to create a grid layout with center and end aligned components
 * to verify that the alignment fixes work correctly.
 */

#include "../presentation/ui/declarative/Layouts.h"
#include "../presentation/ui/widgets/UiText.h"

namespace UI {

// Example function that creates a test grid with different alignments
std::unique_ptr<IUiComponent> createAlignmentTestGrid() {
    Grid grid;
    
    // Set up a 3x3 grid with fixed 200x100 pixel cells
    grid.setRows({
        Grid::Track{Grid::TrackType::Pixel, 100.0f},
        Grid::Track{Grid::TrackType::Pixel, 100.0f}, 
        Grid::Track{Grid::TrackType::Pixel, 100.0f}
    });
    grid.setCols({
        Grid::Track{Grid::TrackType::Pixel, 200.0f},
        Grid::Track{Grid::TrackType::Pixel, 200.0f},
        Grid::Track{Grid::TrackType::Pixel, 200.0f}
    });
    
    // Row 1: Start, Center, End horizontal alignment (all centered vertically)
    auto textStart = std::make_unique<Text>();
    textStart->setText("Start");
    grid.addItem(Grid::Item{
        .row = 0, .col = 0,
        .h = Grid::CellAlign::Start, .v = Grid::CellAlign::Center,
        .widget = std::move(textStart)
    });
    
    auto textCenter = std::make_unique<Text>();
    textCenter->setText("Center");
    grid.addItem(Grid::Item{
        .row = 0, .col = 1,
        .h = Grid::CellAlign::Center, .v = Grid::CellAlign::Center,
        .widget = std::move(textCenter)
    });
    
    auto textEnd = std::make_unique<Text>();
    textEnd->setText("End");
    grid.addItem(Grid::Item{
        .row = 0, .col = 2,
        .h = Grid::CellAlign::End, .v = Grid::CellAlign::Center,
        .widget = std::move(textEnd)
    });
    
    // Row 2: All center horizontally, Start/Center/End vertically
    auto textTop = std::make_unique<Text>();
    textTop->setText("Top");
    grid.addItem(Grid::Item{
        .row = 1, .col = 0,
        .h = Grid::CellAlign::Center, .v = Grid::CellAlign::Start,
        .widget = std::move(textTop)
    });
    
    auto textMiddle = std::make_unique<Text>();
    textMiddle->setText("Middle");
    grid.addItem(Grid::Item{
        .row = 1, .col = 1,
        .h = Grid::CellAlign::Center, .v = Grid::CellAlign::Center,
        .widget = std::move(textMiddle)
    });
    
    auto textBottom = std::make_unique<Text>();
    textBottom->setText("Bottom");
    grid.addItem(Grid::Item{
        .row = 1, .col = 2,
        .h = Grid::CellAlign::Center, .v = Grid::CellAlign::End,
        .widget = std::move(textBottom)
    });
    
    // Row 3: Stretch examples
    auto textStretch = std::make_unique<Text>();
    textStretch->setText("Stretched");
    grid.addItem(Grid::Item{
        .row = 2, .col = 1,
        .h = Grid::CellAlign::Stretch, .v = Grid::CellAlign::Stretch,
        .widget = std::move(textStretch)
    });
    
    return grid.build();
}

/*
 * Expected behavior after the fix:
 * 
 * Row 1:
 * - "Start" should appear at the left edge of its cell
 * - "Center" should appear exactly centered in its cell (NOT at the left edge)
 * - "End" should appear at the right edge of its cell (NOT with 1px gap)
 * 
 * Row 2:
 * - "Top" should appear horizontally centered, at the top edge
 * - "Middle" should appear exactly centered both horizontally and vertically
 * - "Bottom" should appear horizontally centered, at the bottom edge (NOT with 1px gap)
 * 
 * Row 3:
 * - "Stretched" should fill the entire cell
 * 
 * Before the fix:
 * - "Center" items would stick to the Start edge due to full-width measurement
 * - "End" items might have a 1px offset due to QRect::right()/bottom() usage
 */

}