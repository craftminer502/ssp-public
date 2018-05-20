#ifndef FORM_H
 #define FORM_H

 #include <QWidget>

 namespace Ui {
  class Form;
 }

 class Form : public QWidget
 {
  Q_OBJECT

 public:
  explicit Form(QWidget *parent = 0);
  ~Form();

 void setName(const QString &name);
  QString name() const;

 private:
  Ui::Form *ui;
 };

 #endif // FORM_H
