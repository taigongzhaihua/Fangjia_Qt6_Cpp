/*
 * Direct Core Popup Usage Example
 * 
 * Demonstrates how to use the core Popup class directly without the UI wrapper,
 * showing the new external control pattern where triggers are managed separately.
 */

#include "presentation/ui/widgets/Popup.h"
#include "presentation/ui/base/UiComponent.hpp"
#include <QApplication>
#include <QMainWindow>
#include <QPushButton>
#include <QVBoxLayout>
#include <QLabel>
#include <QDebug>
#include <memory>

class DirectPopupExample : public QMainWindow
{
    Q_OBJECT

public:
    DirectPopupExample(QWidget* parent = nullptr) : QMainWindow(parent)
    {
        setupUI();
        setupPopup();
    }

private slots:
    void onTriggerClicked()
    {
        QRect buttonGeometry = m_triggerButton->geometry();
        
        if (m_popup->isPopupVisible()) {
            qDebug() << "Hiding popup via external trigger";
            m_popup->hidePopup();
        } else {
            qDebug() << "Showing popup at trigger position";
            // Use the new external positioning method
            m_popup->showPopupAtPosition(buttonGeometry);
        }
    }
    
    void onShowAtCenterClicked()
    {
        qDebug() << "Showing popup at center";
        // Direct positioning control
        QPoint centerPos = rect().center() - QPoint(100, 75); // Center roughly
        m_popup->showPopupAt(centerPos);
    }
    
    void onHideClicked()
    {
        qDebug() << "Hiding popup via external control";
        m_popup->hidePopup();
    }

private:
    void setupUI()
    {
        auto* centralWidget = new QWidget(this);
        setCentralWidget(centralWidget);

        auto* layout = new QVBoxLayout(centralWidget);
        
        auto* titleLabel = new QLabel("Direct Core Popup Usage Example", this);
        titleLabel->setAlignment(Qt::AlignCenter);
        titleLabel->setStyleSheet("font-size: 16px; font-weight: bold; margin: 10px;");
        layout->addWidget(titleLabel);

        // Primary trigger button
        m_triggerButton = new QPushButton("Main Trigger (Toggle Popup)", this);
        connect(m_triggerButton, &QPushButton::clicked, this, &DirectPopupExample::onTriggerClicked);
        layout->addWidget(m_triggerButton);

        // Additional control buttons demonstrating external control
        auto* centerButton = new QPushButton("Show at Center", this);
        connect(centerButton, &QPushButton::clicked, this, &DirectPopupExample::onShowAtCenterClicked);
        layout->addWidget(centerButton);
        
        auto* hideButton = new QPushButton("Hide Popup", this);
        connect(hideButton, &QPushButton::clicked, this, &DirectPopupExample::onHideClicked);
        layout->addWidget(hideButton);

        // Status info
        auto* infoLabel = new QLabel(
            "This example shows direct usage of the core Popup class:\n"
            "• Popup maintains only open/close state\n"
            "• External controls decide when to show/hide\n"
            "• Position can be controlled externally\n"
            "• No built-in trigger functionality"
        , this);
        infoLabel->setWordWrap(true);
        infoLabel->setStyleSheet("color: gray; margin: 10px; font-size: 12px;");
        layout->addWidget(infoLabel);

        setWindowTitle("Direct Core Popup Example");
        setGeometry(200, 200, 400, 300);
    }

    void setupPopup()
    {
        // Create popup with just the parent window - no trigger
        m_popup = std::make_unique<Popup>(windowHandle());
        
        // Configure popup properties
        m_popup->setPopupSize(QSize(200, 150));
        m_popup->setPlacement(Popup::Placement::Bottom);
        m_popup->setBackgroundColor(QColor(255, 255, 255, 240));
        m_popup->setCornerRadius(8.0f);
        m_popup->setOffset(QPoint(0, 5));
        
        // Set up visibility change callback
        m_popup->setOnVisibilityChanged([this](bool visible) {
            qDebug() << "Popup visibility changed:" << visible;
            m_statusLabel->setText(visible ? "Status: Popup Open" : "Status: Popup Closed");
        });

        // Create popup content (this would normally use a UI component)
        // For this example, we'll create a simple text content
        // In a real application, you'd create an IUiComponent for the content
        
        // Add status label to track popup state
        m_statusLabel = new QLabel("Status: Popup Closed", this);
        m_statusLabel->setAlignment(Qt::AlignCenter);
        centralWidget()->layout()->addWidget(m_statusLabel);
    }

private:
    std::unique_ptr<Popup> m_popup;
    QPushButton* m_triggerButton = nullptr;
    QLabel* m_statusLabel = nullptr;
};

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    DirectPopupExample window;
    window.show();

    qDebug() << "Direct Core Popup Example started";
    qDebug() << "Demonstrating the new popup architecture:";
    qDebug() << "1. Core Popup class has no built-in trigger";
    qDebug() << "2. External controls manage popup state";
    qDebug() << "3. Position can be controlled externally";
    qDebug() << "4. Multiple triggers can control same popup";

    return app.exec();
}

#include "direct_popup_example.moc"