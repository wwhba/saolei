#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QPushButton>
#include <QLabel>
#include <QTimer>
#include <QComboBox>
#include <QSignalMapper>
#include <vector>
#include "timerecorder.h"

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = 0);
    ~MainWindow();

private slots:
    void onResetButtonClicked();
    void onDifficultyChanged(int index);
    void updateTimer();
    void onButtonClicked(int position);
    void onRightClick(int position);
    void onChallengeButtonClicked();  // 挑战按钮点击事件
    void updateChallengeTimer();
        void startChallenge(int seconds);
private:
    struct Cell {
        bool isMine;
        bool isRevealed;
        bool isFlagged;
        int adjacentMines;
        QPushButton* button;
    };

    enum Difficulty {
        Beginner,
        Intermediate,
        Expert
    };

    std::vector<std::vector<Cell> > board;
    QTimer* timer;
    int secondsElapsed;
    bool gameOver;
    bool gameStarted;
    int rows, cols, numMines;
    Difficulty currentDifficulty;

    int firstClickRow, firstClickCol;     // 记录首次点击的行列位置
    bool isFirstClick;                      // 标记是否为首次点击
    // UI 控件
    QComboBox *difficultyCombo;
    QLabel *mineCountLabel;
    QLabel *timeLabel;
    QPushButton *resetButton;
    QWidget *centralWidget;
    QSignalMapper *leftClickMapper;
    QSignalMapper *rightClickMapper;

    // 新增时间记录器
    TimeRecorder* timeRecorder;
    QPushButton *challengeButton;     // 挑战按钮
        QTimer *challengeTimer;           // 挑战模式计时器
        int challengeSecondsRemaining;    // 挑战剩余时间
        bool isChallengeMode;
    void setupUI();
    void setDifficulty(Difficulty diff);
    void initBoard();
    void placeMines();
    void calculateAdjacentMines();
    void revealCell(int row, int col);
    void revealAllMines();
    void checkGameStatus();
    void resetGame();
    void updateMineCount();
    QString getDifficultyString() const;  // 新增方法
};

#endif // MAINWINDOW_H
