#include "mainwindow.h"
#include <QApplication>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QMessageBox>
#include <QDesktopWidget>
#include <cstdlib>
#include <ctime>
#include <QInputDialog>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent),
      timer(new QTimer(this)),
      secondsElapsed(0),
      gameOver(false),
      gameStarted(false),
      currentDifficulty(Beginner),
      leftClickMapper(new QSignalMapper(this)),
      rightClickMapper(new QSignalMapper(this)),
      isFirstClick(true),
      timeRecorder(new TimeRecorder(this)),
    challengeTimer(nullptr),
          challengeSecondsRemaining(0),
          isChallengeMode(false)
{
    setWindowTitle("扫雷");
    setWindowFlags(windowFlags() & ~Qt::WindowMaximizeButtonHint);
    setupUI();

    connect(difficultyCombo, SIGNAL(currentIndexChanged(int)), this, SLOT(onDifficultyChanged(int)));
    connect(resetButton, SIGNAL(clicked()), this, SLOT(onResetButtonClicked()));
    connect(timer, SIGNAL(timeout()), this, SLOT(updateTimer()));
    connect(leftClickMapper, SIGNAL(mapped(int)), this, SLOT(onButtonClicked(int)));
    connect(rightClickMapper, SIGNAL(mapped(int)), this, SLOT(onRightClick(int)));

    setDifficulty(Beginner);
    resetGame();

    adjustSize();

    QDesktopWidget *desktop = QApplication::desktop();
    int screenWidth = desktop->width();
    int screenHeight = desktop->height();
    int windowWidth = width();
    int windowHeight = height();
    setGeometry(
        (screenWidth - windowWidth) / 2,
        (screenHeight - windowHeight) / 2,
        windowWidth,
        windowHeight
    );
}

MainWindow::~MainWindow()
{
}

void MainWindow::setupUI()
{
    centralWidget = new QWidget(this);
    setCentralWidget(centralWidget);

    QVBoxLayout *mainLayout = new QVBoxLayout(centralWidget);
    mainLayout->setSpacing(10);
    mainLayout->setMargin(10);

    QWidget *topWidget = new QWidget(this);
    QHBoxLayout *topLayout = new QHBoxLayout(topWidget);
    topLayout->setSpacing(10);
    topLayout->setMargin(0);

    difficultyCombo = new QComboBox(this);
    difficultyCombo->addItem("初级");
    difficultyCombo->addItem("中级");
    difficultyCombo->addItem("高级");
    topLayout->addWidget(difficultyCombo);

    mineCountLabel = new QLabel("000", this);
    mineCountLabel->setFixedWidth(50);
    topLayout->addWidget(mineCountLabel);

    resetButton = new QPushButton("🙂", this);
    resetButton->setFixedSize(30, 30);
    topLayout->addWidget(resetButton);

    timeLabel = new QLabel("000", this);
    timeLabel->setFixedWidth(50);
    topLayout->addWidget(timeLabel);

    QPushButton* recordsButton = new QPushButton("查看记录", this);
    topLayout->addWidget(recordsButton);
    connect(recordsButton, &QPushButton::clicked, this, &MainWindow::onRecordsButtonClicked);

    challengeButton = new QPushButton("挑战", this);
    topLayout->addWidget(challengeButton);
    connect(challengeButton, &QPushButton::clicked, this, &MainWindow::onChallengeButtonClicked);

    mainLayout->addWidget(topWidget);

    QGridLayout *gridLayout = new QGridLayout();
    gridLayout->setHorizontalSpacing(0);
    gridLayout->setVerticalSpacing(0);
    gridLayout->setMargin(0);
    gridLayout->setSizeConstraint(QLayout::SetFixedSize);
    mainLayout->addLayout(gridLayout);


}
void MainWindow::onChallengeButtonClicked()
{
    bool ok;
    int seconds = QInputDialog::getInt(
        this,
        "设置挑战时间",
        "请输入挑战时间（秒）:",
        60,
        10,
        1000,
        1,
        &ok
    );

    if (ok && seconds > 0) {
        startChallenge(seconds);
    }
}
void MainWindow::startChallenge(int seconds)
{
    resetGame();
    isChallengeMode = true;
    challengeSecondsRemaining = seconds;

    timeLabel->setText(QString("%1").arg(challengeSecondsRemaining, 3, 10, QChar('0')));

    if (!challengeTimer) {
        challengeTimer = new QTimer(this);
        connect(challengeTimer, &QTimer::timeout, this, &MainWindow::updateChallengeTimer);
    }
    challengeTimer->start(1000);
}
void MainWindow::updateChallengeTimer()
{
    if (challengeSecondsRemaining > 0) {
        challengeSecondsRemaining--;
        timeLabel->setText(QString("%1").arg(challengeSecondsRemaining, 3, 10, QChar('0')));

        if (challengeSecondsRemaining <= 10) {
            timeLabel->setStyleSheet("color: red; font-weight: bold;");
        }
    } else {
        challengeTimer->stop();
        gameOver = true;
        revealAllMines();
        resetButton->setText("😞");
        QMessageBox::critical(this, "挑战失败", "时间已用完！");
    }
}
void MainWindow::setDifficulty(Difficulty diff)
{
    currentDifficulty = diff;
    switch (diff) {
        case Beginner:
            rows = 9; cols = 9; numMines = 10;
            break;
        case Intermediate:
            rows = 16; cols = 16; numMines = 40;
            break;
        case Expert:
            rows = 16; cols = 30; numMines = 99;
            break;
    }
}

void MainWindow::initBoard()
{
    QGridLayout *gridLayout = dynamic_cast<QGridLayout*>(centralWidget->layout()->itemAt(1));
    if (!gridLayout) return;

    while (gridLayout->count() > 0) {
        QLayoutItem* item = gridLayout->takeAt(0);
        if (QWidget* widget = item->widget()) {
            delete widget;
        }
        delete item;
    }

    board.clear();
    board.resize(rows, std::vector<Cell>(cols));

    int btnSize = 50;
    for (int i = 0; i < rows; ++i) {
        for (int j = 0; j < cols; ++j) {
            board[i][j].button = new QPushButton(this);
            board[i][j].button->setFixedSize(btnSize, btnSize);
            board[i][j].button->setStyleSheet("border: 1px solid gray; background-color: #ccc;");
            gridLayout->addWidget(board[i][j].button, i, j);

            int position = i * cols + j;
            connect(board[i][j].button, SIGNAL(clicked()), leftClickMapper, SLOT(map()));
            leftClickMapper->setMapping(board[i][j].button, position);

            board[i][j].button->setContextMenuPolicy(Qt::CustomContextMenu);
            connect(board[i][j].button, SIGNAL(customContextMenuRequested(const QPoint&)), rightClickMapper, SLOT(map()));
            rightClickMapper->setMapping(board[i][j].button, position);

            board[i][j].isMine = false;
            board[i][j].isRevealed = false;
            board[i][j].isFlagged = false;
            board[i][j].adjacentMines = 0;
        }
    }

    updateMineCount();
    centralWidget->layout()->activate();
    adjustSize();
}


void MainWindow::calculateAdjacentMines()
{
    for (int i = 0; i < rows; ++i) {
        for (int j = 0; j < cols; ++j) {
            if (board[i][j].isMine) continue;

            int count = 0;
            for (int x = qMax(0, i-1); x <= qMin(rows-1, i+1); ++x) {
                for (int y = qMax(0, j-1); y <= qMin(cols-1, j+1); ++y) {
                    if (board[x][y].isMine) count++;
                }
            }
            board[i][j].adjacentMines = count;
        }
    }
}

void MainWindow::revealCell(int row, int col)
{
    if (row < 0 || row >= rows || col < 0 || col >= cols ||
        board[row][col].isRevealed || board[row][col].isFlagged)
        return;

    board[row][col].isRevealed = true;
    board[row][col].button->setEnabled(false);

    if (board[row][col].isMine) {
        board[row][col].button->setText("*");
        board[row][col].button->setStyleSheet("background-color: red; color: black;");
        revealAllMines();
        gameOver = true;
        timer->stop();
        resetButton->setText("😞");
        QMessageBox::critical(this, "游戏结束", "踩到地雷了！");
        return;
    }

    board[row][col].button->setStyleSheet("border: 1px solid #888; background-color: #eee;");

    if(board[row][col].adjacentMines > 0) {
        board[row][col].button->setText(QString::number(board[row][col].adjacentMines));
        QString color;
        switch (board[row][col].adjacentMines) {
            case 1: color = "blue"; break;
            case 2: color = "green"; break;
            case 3: color = "red"; break;
            case 4: color = "darkblue"; break;
            case 5: color = "darkred"; break;
            case 6: color = "cyan"; break;
            case 7: color = "black"; break;
            case 8: color = "gray"; break;
            default: color = "black";
        }
        board[row][col].button->setStyleSheet(QString("color: %1; font-size: 20px;").arg(color));
    } else {
        for (int x = qMax(0, row-1); x <= qMin(rows-1, row+1); ++x) {
            for (int y = qMax(0, col-1); y <= qMin(cols-1, col+1); ++y) {
                revealCell(x, y);
            }
        }
    }
}

void MainWindow::revealAllMines()
{
    for (int i = 0; i < rows; ++i) {
        for (int j = 0; j < cols; ++j) {
            if (board[i][j].isMine) {
                board[i][j].button->setText("*");
                if (!board[i][j].isFlagged)
                    board[i][j].button->setStyleSheet("background-color: #faa;");
            } else if (board[i][j].isFlagged) {
                board[i][j].button->setText("X");
                board[i][j].button->setStyleSheet("background-color: #fc6;");
            }
        }
    }
}

void MainWindow::checkGameStatus()
{
    bool allNonMinesRevealed = true;
    for (int i = 0; i < rows; ++i) {
        for (int j = 0; j < cols; ++j) {
            if (!board[i][j].isMine && !board[i][j].isRevealed) {
                allNonMinesRevealed = false;
                break;
            }
        }
        if (!allNonMinesRevealed) break;
    }

    if (allNonMinesRevealed) {
        for (int i = 0; i < rows; ++i) {
            for (int j = 0; j < cols; ++j) {
                if (board[i][j].isMine) {
                    board[i][j].isFlagged = true;
                    board[i][j].button->setText("F");
                    board[i][j].button->setStyleSheet("background-color: #cfc; color: green;");
                }
            }
        }
        gameOver = true;

        if (isChallengeMode) {
            challengeTimer->stop();

            timeRecorder->addRecord(challengeSecondsRemaining, getDifficultyString() + " (挑战模式)");

            QMessageBox::information(this, "挑战成功",
                QString("恭喜你在挑战时间内完成！剩余时间: %1 秒\n\n难度: %2")
                    .arg(challengeSecondsRemaining)
                    .arg(getDifficultyString()));
        } else {
            timer->stop();
            timeRecorder->addRecord(secondsElapsed, getDifficultyString());

            QMessageBox::information(this, "游戏胜利",
                QString("恭喜你赢了！用时: %1 秒\n\n难度: %2")
                    .arg(secondsElapsed)
                    .arg(getDifficultyString()));
        }
    }
}

QString MainWindow::getDifficultyString() const
{
    switch (currentDifficulty) {
        case Beginner: return "初级";
        case Intermediate: return "中级";
        case Expert: return "高级";
        default: return "未知";
    }
}

void MainWindow::updateTimer()
{
    secondsElapsed++;
    timeLabel->setText(QString("%1").arg(secondsElapsed, 3, 10, QChar('0')));
    if (secondsElapsed >= 999) timer->stop();
}

void MainWindow::updateMineCount()
{
    int flagged = 0;
    for (int i = 0; i < rows; ++i) {
        for (int j = 0;j < cols; ++j) {
            if (board[i][j].isFlagged) flagged++;
        }
    }
    int remaining = numMines - flagged;
    mineCountLabel->setText(QString("%1").arg(remaining, 3, 10, QChar('0')));
}


void MainWindow::onButtonClicked(int position)
{
    int row = position / cols;
    int col = position % cols;

    if (gameOver || board[row][col].isRevealed || board[row][col].isFlagged) return;

    if (!gameStarted) {
        gameStarted = true;
        firstClickRow = row;
        firstClickCol = col;

        if (isChallengeMode) {
            challengeTimer->start(1000);
        } else {
            timer->start(1000);
        }


        placeMinesWithSafety();
        calculateAdjacentMines();
    }

    revealCell(row, col);
    checkGameStatus();
}
void MainWindow::placeMinesWithSafety()
{

    int startRow = qMax(0, firstClickRow - 1);
    int endRow = qMin(rows - 1, firstClickRow + 1);
    int startCol = qMax(0, firstClickCol - 1);
    int endCol = qMin(cols - 1, firstClickCol + 1);

    int minesPlaced = 0;
    while (minesPlaced < numMines) {
        int row = rand() % rows;
        int col = rand() % cols;

        if (row >= startRow && row <= endRow && col >= startCol && col <= endCol) {
            continue;
        }
        if (!board[row][col].isMine) {
            board[row][col].isMine = true;
            minesPlaced++;
        }
    }
}
void MainWindow::onRightClick(int position)
{
    int row = position / cols;
    int col = position % cols;

    if (gameOver || board[row][col].isRevealed) return;

    board[row][col].isFlagged = !board[row][col].isFlagged;
    board[row][col].button->setText(board[row][col].isFlagged ? "F" : "");
    board[row][col].button->setStyleSheet(board[row][col].isFlagged ?
        "background-color: #fcc;" : "border: 1px solid gray; background-color: #ccc;");
    updateMineCount();
    checkGameStatus();
}

void MainWindow::onResetButtonClicked()
{
    resetGame();
}

void MainWindow::onDifficultyChanged(int index)
{
    setDifficulty(static_cast<Difficulty>(index));
    resetGame();
}

void MainWindow::resetGame()
{
    timer->stop();
    if (challengeTimer) challengeTimer->stop();

    secondsElapsed = 0;
    timeLabel->setText("000");
    timeLabel->setStyleSheet("");

    gameOver = false;
    gameStarted = false;
    resetButton->setText("🙂");
    isChallengeMode = false;

    initBoard();

    isFirstClick = true;
        gameStarted = false;
}
RecordsDialog::RecordsDialog(const QList<TimeRecord>& records, QWidget* parent)
    : QDialog(parent)
{
    setWindowTitle("游戏记录");
    setFixedSize(200, 300);

    QVBoxLayout* mainLayout = new QVBoxLayout(this);

    QScrollArea* scrollArea = new QScrollArea(this);
    scrollArea->setWidgetResizable(true);
    scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    scrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);

    QWidget* scrollWidget = new QWidget(scrollArea);
    QVBoxLayout* scrollLayout = new QVBoxLayout(scrollWidget);
    scrollLayout->setSpacing(5);
    scrollLayout->setMargin(10);

    if (records.isEmpty()) {
        QLabel* emptyLabel = new QLabel("暂无记录", scrollWidget);
        emptyLabel->setAlignment(Qt::AlignCenter);
        emptyLabel->setStyleSheet("color: #999;");
        scrollLayout->addWidget(emptyLabel);
    } else {
        for (const auto& record : records) {
            QString timeStr = QString("%1秒").arg(record.seconds);
            /*QString dateStr = record.date.toString("yyyy-MM-dd HH:mm:ss");*/
            QString difficultyStr = record.difficulty;

            QString recordText = QString("%1-%3")
                .arg(timeStr)
                /*.arg(dateStr)*/
                .arg(difficultyStr);

            QLabel* recordLabel = new QLabel(recordText, scrollWidget);
            recordLabel->setStyleSheet("border-bottom: 1px solid #eee; padding-bottom: 5px;");
            scrollLayout->addWidget(recordLabel);
        }
    }

    scrollWidget->setLayout(scrollLayout);
    scrollArea->setWidget(scrollWidget);
    mainLayout->addWidget(scrollArea);

    QHBoxLayout* buttonLayout = new QHBoxLayout();
    QPushButton* clearButton = new QPushButton("清除所有记录", this);
    connect(clearButton, &QPushButton::clicked, this, [this]() {
        emit clearRecordsRequested();
    });
    buttonLayout->addWidget(clearButton);
    mainLayout->addLayout(buttonLayout);

    connect(this, &RecordsDialog::clearRecordsRequested,
            [this]() { accept(); });
}

void MainWindow::onRecordsButtonClicked()
{

    QList<TimeRecord> records = timeRecorder->getSortedRecords();
    RecordsDialog* dialog = new RecordsDialog(records, this);

    connect(dialog, &RecordsDialog::clearRecordsRequested, [this]() {
        timeRecorder->clearRecords();
        QMessageBox::information(this, "提示", "记录已全部清除");
    });

    dialog->exec();
}
void MainWindow::clearRecords()
{
    int result = QMessageBox::question(
        this,
        "确认清除",
        "确定要清除所有游戏记录吗？此操作不可恢复。",
        QMessageBox::Yes | QMessageBox::No,
        QMessageBox::No
    );

    if (result == QMessageBox::Yes) {
        // 调用 TimeRecorder 清除记录
        timeRecorder->clearRecords();

        // 显示成功提示
        QMessageBox::information(
            this,
            "操作成功",
            "所有游戏记录已清除。"
        );
    }
}
