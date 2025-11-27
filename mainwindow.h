#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QGraphicsScene>
#include <QMap>
#include <QVector>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

struct Vertex {
    int index;
    double x, y, z;
    QVector<int> adj;
    QVector<double> weights;
};

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

protected:
    bool eventFilter(QObject *watched, QEvent *event) override;

private slots:
    void on_importButton_clicked();
    void on_buildButton_clicked();

private:
    Ui::MainWindow *ui;
    QGraphicsScene *scene = nullptr;

    QMap<int, Vertex> graph; // index -> Vertex
    QMap<QPair<int,int>, QGraphicsLineItem*> edgeItems; // normalized pair -> line
    QMap<int, QGraphicsEllipseItem*> nodeItems; // index -> ellipse

    int startIndex = -1;
    int goalIndex = -1;

    // Zoom / Pan state
    double currentScale = 1.0;
    const double zoomStep = 1.15;
    const double minScale = 0.1;
    const double maxScale = 10.0;
    bool panning = false;
    QPoint lastPanPoint;

    // Для асинхронной паузы без зависания интерфейса. Нужна для "красивого" выделения цветом статусбара
    void pause(int ms);

    bool parseJsonFile(const QString &path);
    void drawGraph();
    void clearScene();
    void clearSelectionColors();
    void highlightPath(const QVector<int> &path);

    // Алгосы
    QVector<int> a_star(int start, int goal);
    QVector<int> dijkstra(int start, int goal);

    // Функции для маштобирования сцены
    void zoomAt(const QPointF &scenePos, double factor);
    void resetViewToFit();

    // подложка
    QGraphicsPixmapItem *bgItem = nullptr;   // элемент подложки на сцене
    QPixmap bgPixmap;                        // оригинальный pixmap (чтобы перестраивать при redraw)
    bool bgKeepAspect = true;                // флаг: сохранять аспект при масштабировании
    // слот (объявление)
private slots:
    void on_loadBgButton_clicked();

    //void on_comboBox_currentTextChanged(const QString &arg1);

    void ChoiseDijkstra();

    void ChoiseA_star();

    void ChoiseAnt();

    void SetLightTema();
    void SetDarkTema();
};

#endif // MAINWINDOW_H
