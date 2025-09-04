/*
 * Example showing external control of popup without built-in trigger
 * 
 * This demonstrates the new architecture where popup only maintains
 * open/close state and external controls manage when to show/hide.
 */

#include <QApplication>
#include <QMainWindow>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QWidget>
#include <QPushButton>
#include <QLabel>
#include <QDebug>

// This example would use the UI framework, but shows the concept
class ExternalControlExample : public QMainWindow
{
    Q_OBJECT

public:
    ExternalControlExample(QWidget* parent = nullptr) : QMainWindow(parent)
    {
        setupUI();
    }

private slots:
    void togglePopup()
    {
        // External control - trigger decides when to show/hide popup
        if (m_isPopupOpen) {
            qDebug() << "Hiding popup via external control";
            hidePopup();
        } else {
            qDebug() << "Showing popup via external control";
            showPopup();
        }
    }

private:
    void setupUI()
    {
        auto* centralWidget = new QWidget(this);
        setCentralWidget(centralWidget);

        auto* layout = new QVBoxLayout(centralWidget);
        
        auto* label = new QLabel("External Popup Control Example", this);
        label->setAlignment(Qt::AlignCenter);
        layout->addWidget(label);

        // Primary trigger button
        auto* triggerButton = new QPushButton("Toggle Popup", this);
        connect(triggerButton, &QPushButton::clicked, this, &ExternalControlExample::togglePopup);
        layout->addWidget(triggerButton);

        // Additional controls - any widget can control the popup
        auto* controlsLayout = new QHBoxLayout();
        
        auto* showButton = new QPushButton("Show", this);
        connect(showButton, &QPushButton::clicked, this, &ExternalControlExample::showPopup);
        controlsLayout->addWidget(showButton);
        
        auto* hideButton = new QPushButton("Hide", this);
        connect(hideButton, &QPushButton::clicked, this, &ExternalControlExample::hidePopup);
        controlsLayout->addWidget(hideButton);

        layout->addLayout(controlsLayout);

        // Status label
        m_statusLabel = new QLabel("Popup State: Closed", this);
        m_statusLabel->setAlignment(Qt::AlignCenter);
        layout->addWidget(m_statusLabel);

        setWindowTitle("External Popup Control Example");
        setGeometry(100, 100, 300, 200);
    }

    void showPopup()
    {
        if (!m_isPopupOpen) {
            m_isPopupOpen = true;
            m_statusLabel->setText("Popup State: Open");
            
            // In the actual implementation, this would call:
            // popup->showPopupAt(calculatePosition());
            // or popup->showPopupAtPosition(triggerButton->geometry());
            qDebug() << "Popup opened externally";
        }
    }

    void hidePopup()
    {
        if (m_isPopupOpen) {
            m_isPopupOpen = false;
            m_statusLabel->setText("Popup State: Closed");
            
            // In the actual implementation, this would call:
            // popup->hidePopup();
            qDebug() << "Popup closed externally";
        }
    }

private:
    bool m_isPopupOpen = false;
    QLabel* m_statusLabel = nullptr;
};

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    ExternalControlExample window;
    window.show();

    qDebug() << "External Popup Control Example started";
    qDebug() << "This demonstrates the new popup architecture:";
    qDebug() << "1. Popup only maintains open/close state";
    qDebug() << "2. External controls decide when to show/hide";
    qDebug() << "3. Multiple triggers can control the same popup";
    qDebug() << "4. Flexible event-driven control";

    return app.exec();
}

#include "popup_external_control_example.moc"