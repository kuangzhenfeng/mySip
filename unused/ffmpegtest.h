#ifndef FFMPEGTEST_H
#define FFMPEGTEST_H

#include <QWidget>


namespace Ui {
class FfmpegTest;
}

class FfmpegTest : public QWidget
{
    Q_OBJECT

public:
    explicit FfmpegTest(QWidget *parent = nullptr);
    ~FfmpegTest();

private:
    Ui::FfmpegTest *ui;
};

#endif // FFMPEGTEST_H
