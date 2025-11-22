#pragma once
#include <QPushButton>
#include <QHBoxLayout>

#include <QWidget>
#include <QLabel>
#include <QIcon>


class NavBar : public QWidget
{
    Q_OBJECT
public:
    explicit NavBar(QWidget* parent = nullptr);

signals:
    void homeClicked();
    void ticketClicked();
    void idClicked();

private:
    QPushButton* homeButton_ = nullptr;
    QPushButton* ticketButton_ = nullptr;
    QPushButton* idButton_ = nullptr;
};