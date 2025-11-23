#pragma once
#include <QWidget>
#include <QLineEdit>
#include <QDateEdit>
#include <QTimeEdit>
#include <QPushButton>

class LocationSelection : public QWidget
{
    Q_OBJECT
public:
    explicit LocationSelection(QWidget* parent = nullptr);
    void clear();

signals:
    void searchClicked(const QString& departure, const QString& destination, const QDate& date, const QTime& time);

private:
    QLineEdit* departureEdit_ = nullptr;
    QLineEdit* destinationEdit_ = nullptr;
    QDateEdit* dateEdit_ = nullptr;
    QTimeEdit* timeEdit_ = nullptr;
    QPushButton* goButton_ = nullptr;
};