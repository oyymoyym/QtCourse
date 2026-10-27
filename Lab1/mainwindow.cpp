#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <math.h>
#include <QDebug>
#include <QKeyEvent>
#include <QPushButton>

    MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    digitBTNs = {{Qt::Key_0, ui->btnNum0},
                 {Qt::Key_1, ui->btnNum1},
                 {Qt::Key_2, ui->btnNum2},
                 {Qt::Key_3, ui->btnNum3},
                 {Qt::Key_4, ui->btnNum4},
                 {Qt::Key_5, ui->btnNum5},
                 {Qt::Key_6, ui->btnNum6},
                 {Qt::Key_7, ui->btnNum7},
                 {Qt::Key_8, ui->btnNum8},
                 {Qt::Key_9, ui->btnNum9},
                 };
    foreach(auto btn, digitBTNs)
        connect(btn, SIGNAL(clicked()), this, SLOT(btnNumClicked()));
    connect(ui->btnPlus, SIGNAL(clicked()), this, SLOT(btnBinaryOperatorClicked()));
    connect(ui->btnMinus, SIGNAL(clicked()), this, SLOT(btnBinaryOperatorClicked()));
    connect(ui->btnMultiple, SIGNAL(clicked()), this, SLOT(btnBinaryOperatorClicked()));
    connect(ui->btnDivide, SIGNAL(clicked()), this, SLOT(btnBinaryOperatorClicked()));
    connect(ui->btnPercentage, SIGNAL(clicked()), this, SLOT(btnUnaryOperatorClicked()));
    connect(ui->btnInverse, SIGNAL(clicked()), this, SLOT(btnUnaryOperatorClicked()));
    connect(ui->btnSquare, SIGNAL(clicked()), this, SLOT(btnUnaryOperatorClicked()));
    connect(ui->btnSqrt, SIGNAL(clicked()), this, SLOT(btnUnaryOperatorClicked()));
}

MainWindow::~MainWindow()
{
    delete ui;
}

QString MainWindow::calculation(bool *ok)
{
    if (ok) *ok = false;

    double result = 0;
    if (operands.size() == 2 && opcodes.size() > 0) {
        double operand1 = operands.front().toDouble();
        operands.pop_front();
        double operand2 = operands.front().toDouble();
        operands.pop_front();

        QString op = opcodes.front();
        opcodes.pop_front();

        if (op == "+") {
            result = operand1 + operand2;
        } else if (op == "-") {
            result = operand1 - operand2;
        } else if (op == "×") {
            result = operand1 * operand2;
        } else if (op == "/") {
            if (operand2 == 0) {
                operands.clear();
                opcodes.clear();
                ui->statusbar->showMessage("错误：除数不能为 0");
                return "Error";
            }
            result = operand1 / operand2;
        }

        operands.push_back(QString::number(result));
        if (ok) *ok = true;
        return QString::number(result);
    }

    return "";
}

void MainWindow::btnNumClicked()
{
    // 如果上一次是按了等号，那么按数字就开始新的计算
    if (calculationFinished) {
        operand.clear();
        operands.clear();
        opcodes.clear();
        calculationFinished = false;
    }

    QString digit = qobject_cast<QPushButton*>(sender())->text();
    if (digit == "0" && operand == "0")
        digit = "";
    if (digit != "0" && operand == "0")
        operand = "";
    operand += digit;
    ui->display->setText(operand);
}

void MainWindow::on_btnPeriod_clicked()
{
    // 如果上一次是按了等号，按小数点开始新的计算
    if (calculationFinished) {
        operand.clear();
        operands.clear();
        opcodes.clear();
        calculationFinished = false;
    }
    if (!operand.contains(".")) {
        if (operand.isEmpty())
            operand = "0";
        operand += qobject_cast<QPushButton*>(sender())->text();
        ui->display->setText(operand);
    }
}

void MainWindow::on_btnDel_clicked()
{
    if (calculationFinished) {
        // 按了等号后按退格，视为开始新一轮编辑
        calculationFinished = false;
    }
    operand = operand.left(operand.length() - 1);
    ui->display->setText(operand);
}

void MainWindow::on_btnClear_clicked()
{
    operand.clear();
    calculationFinished = false;
    ui->display->setText(operand);
}

void MainWindow::btnBinaryOperatorClicked()
{
    // 按了等号后按运算符：基于上一次的结果继续运算
    // 此时 operand 中保存的是上次的结果，直接使用，只需清除标志
    if (calculationFinished) {
        calculationFinished = false;
    }

    ui->statusbar->showMessage("last operand" + operand);
    QString opcode = qobject_cast<QPushButton*>(sender())->text();
    if (operand != "") {
        operands.push_back(operand);
        operand = "";
        opcodes.push_back(opcode);
    }

    bool ok = false;
    QString result = calculation(&ok);

    if (!result.isEmpty()) {
        if (ok) {
            ui->display->setText(result);
        } else {
            operand.clear();
            ui->display->setText("错误");
            calculationFinished = false;
        }
    }
}

void MainWindow::btnUnaryOperatorClicked()
{
    // 按了等号后按一元运算符：对上次结果进行运算
    if (calculationFinished) {
        calculationFinished = false;
    }

    if (operand != "") {
        double result = operand.toDouble();
        operand = "";
        QString op = qobject_cast<QPushButton*>(sender())->text();
        if (op == "%")
            result /= 100.0;
        else if (op == "1/x")
            result = 1 / result;
        else if (op == "x`2")
            result *= result;
        else if (op == "√")
            result = sqrt(result);
        ui->display->setText(QString::number(result));
        operand = QString::number(result);  // 保留结果，方便后续继续运算
    }
}

void MainWindow::on_btnEqual_clicked()
{
    if (operand != "") {
        operands.push_back(operand);
        operand = "";
    }

    bool ok = false;
    QString result = calculation(&ok);

    if (!result.isEmpty()) {
        if (ok) {
            ui->display->setText(result);
            // 关键：计算完成后将结果作为新的 operand，
            // 以便可以继续按运算符基于此结果运算
            operand = result;
            calculationFinished = true;  // 标记刚完成一次计算
        } else {
            operand.clear();
            ui->display->setText("错误");
            calculationFinished = false;
        }
    }
}

// 键盘绑定按键
void MainWindow::keyPressEvent(QKeyEvent *event)
{
    foreach(auto btnKey, digitBTNs.keys()) {
        if (event->key() == btnKey)
            digitBTNs[btnKey]->animateClick(100);
    }
    if (event->key() == Qt::Key_Plus)
    {
        ui->btnPlus->animateClick(100);
        return;
    }
    if (event->key() == Qt::Key_Minus)
    {
        ui->btnMinus->animateClick(100);
        return;
    }
    if (event->key() == Qt::Key_Asterisk)
    {
        ui->btnMultiple->animateClick(100);
        return;
    }
    if (event->key() == Qt::Key_Slash)
    {
        ui->btnDivide->animateClick(100);
        return;
    }
    if (event->key() == Qt::Key_Period)
    {
        ui->btnPeriod->animateClick(100);
        return;
    }
    if (event->key() == Qt::Key_Backspace)
    {
        ui->btnDel->animateClick(100);
        return;
    }
    if (event->key() == Qt::Key_Enter || event->key() == Qt::Key_Return)
    {
        ui->btnEqual->animateClick(100);
        return;
    }
    if (event->key() == Qt::Key_Escape)
    {
        ui->btnClear->animateClick(100);
        return;
    }
}

void MainWindow::on_btnClearAll_clicked()
{
    operand.clear();
    operands.clear();
    opcodes.clear();
    calculationFinished = false;
    ui->display->setText("0");
}

void MainWindow::on_btnSign_clicked()
{
    // 按了等号后按正负号：对上次结果取反
    if (calculationFinished) {
        calculationFinished = false;
    }

    if (operand.isEmpty()) {
        operand = "-";
        ui->display->setText(operand);
        return;
    }
    double num = operand.toDouble();
    num = -num;
    operand = QString::number(num);
    ui->display->setText(operand);
}
