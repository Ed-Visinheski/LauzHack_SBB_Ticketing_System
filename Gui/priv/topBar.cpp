#include "topBar.h"

TopBar::TopBar(QWidget* parent)
    : QWidget(parent)
{
    // Red SSB background
    setStyleSheet("background-color: #ec0001");

    // SBB Banner behind Red Background
    // auto ssbBanner = new QPushButton(QIcon(QString("icons/SBB_logo.svg")), QString{}, this);

    // Layout with equal spacing: stretch segments around and between buttons
    auto layout = new QHBoxLayout(this);
    layout->setContentsMargins(16, 12, 16, 12); // left, top, right, bottom padding
    layout->setSpacing(0);

    // layout->addStretch(1);

    auto ssbBanner = new QLabel(this);
    ssbBanner->setAlignment(Qt::AlignCenter);
    ssbBanner->setFixedSize(48, 48);
    ssbBanner->setStyleSheet("background: transparent; border: none; padding: 0;");

    QPixmap pix("icons/SBB_logo.svg");
    if (!pix.isNull()) {
        ssbBanner->setPixmap(pix.scaled(ssbBanner->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation));
    }

    layout->addWidget(ssbBanner, 0, Qt::AlignCenter);

    setLayout(layout);
}