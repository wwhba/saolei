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
    void onChallengeButtonClicked();
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

    int firstClickRow, firstClickCol;
    bool isFirstClick;

    QComboBox *difficultyCombo;
    QLabel *mineCountLabel;
    QLabel *timeLabel;
    QPushButton *resetButton;
    QWidget *centralWidget;
    QSignalMapper *leftClickMapper;
    QSignalMapper *rightClickMapper;


    TimeRecorder* timeRecorder;
    QPushButton *challengeButton;
        QTimer *challengeTimer;
        int challengeSecondsRemaining;
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
    QString getDifficultyString() const;
};

#endif // MAINWINDOW_H
