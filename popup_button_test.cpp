/*
 * Simple test to reproduce button hover/press issues in popup
 */

#include <QApplication>
#include <QMainWindow>
#include <QVBoxLayout>
#include <QWidget>
#include <QPushButton>
#include <QLabel>
#include <QDebug>

#include "presentation/ui/widgets/Popup.h"
#include "presentation/ui/widgets/UiPushButton.h"
#include "presentation/ui/containers/UiContainer.h"
#include "UI.h"

class PopupButtonTest : public QMainWindow
{
    Q_OBJECT

public:
    PopupButtonTest(QWidget* parent = nullptr) : QMainWindow(parent)
    {
        setupUI();
        setupPopup();
    }

private slots:
    void showTestPopup()
    {
        if (!m_popup) return;
        
        // Show popup near the trigger button
        QRect triggerRect = m_triggerButton->geometry();
        QPoint globalPos = mapToGlobal(triggerRect.bottomLeft());
        
        qDebug() << "Showing popup at:" << globalPos;
        m_popup->showPopupAt(triggerRect);
    }

private:
    void setupUI()
    {
        auto* centralWidget = new QWidget(this);
        setCentralWidget(centralWidget);

        auto* layout = new QVBoxLayout(centralWidget);
        
        auto* titleLabel = new QLabel("Popup Button Hover Test", this);
        titleLabel->setAlignment(Qt::AlignCenter);
        layout->addWidget(titleLabel);

        m_triggerButton = new QPushButton("Show Popup with Button", this);
        connect(m_triggerButton, &QPushButton::clicked, this, &PopupButtonTest::showTestPopup);
        layout->addWidget(m_triggerButton);

        auto* infoLabel = new QLabel(
            "Click to show popup with a button inside.\n"
            "Test if the button inside the popup responds to hover and clicks properly."
        , this);
        infoLabel->setWordWrap(true);
        layout->addWidget(infoLabel);

        setWindowTitle("Popup Button Test");
        setGeometry(200, 200, 400, 200);
    }
    
    void setupPopup()
    {
        using namespace UI;
        
        // Create a popup with a button inside
        m_popup = std::make_unique<Popup>(windowHandle());
        m_popup->setPopupSize(QSize(250, 120));
        m_popup->setPlacement(Popup::Placement::Bottom);
        m_popup->setBackgroundColor(QColor(255, 255, 255, 240));
        m_popup->setCornerRadius(8.0f);
        
        // Create content with a button
        auto container = vbox()
            ->padding(16, 12, 16, 12)
            ->child(
                text("Test popup content")
                    ->fontSize(14)
            )
            ->child(
                pushButton("Test Button")
                    ->variant(UI::PushButton::Variant::Primary)
                    ->onClick([]() {
                        qDebug() << "Popup button clicked!";
                    })
            );
        
        m_popup->setContent(std::move(container));
        
        m_popup->setOnVisibilityChanged([](bool visible) {
            qDebug() << "Popup visibility changed:" << visible;
        });
    }

private:
    QPushButton* m_triggerButton;
    std::unique_ptr<Popup> m_popup;
};

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    PopupButtonTest window;
    window.show();

    qDebug() << "Popup button test started";
    qDebug() << "Click the button to show a popup with a button inside";
    qDebug() << "Test if the button in the popup responds to hover/click properly";

    return app.exec();
}

#include "popup_button_test.moc"