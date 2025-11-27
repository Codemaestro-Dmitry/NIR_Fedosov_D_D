#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QFileDialog>
#include <QFile>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
#include <QJsonValue>
#include <QProgressDialog>
#include <QMessageBox>
#include <QGraphicsEllipseItem>
#include <QGraphicsLineItem>
#include <QMouseEvent>
#include <QWheelEvent>
#include <QScrollBar>
#include <cmath>
#include <queue>
#include <limits>
#include <QFileDialog>
#include <QGraphicsPixmapItem>
#include<QString>
#include<windows.h>
#include<QTimer>
#include<QAction>

QString choise = "Дейкстра"; // Для выбора алгоритма

// Пока что считается Эвклидово расстояние между точками, причем без учета высоты
// тут бы какой-нибудь API подключить, чтобы получать загруженность дорог, но это
// пока что не возможно (по крайней мере я не нашел где получить такие данные)
static double heuristicEuclid(const Vertex &a, const Vertex &b)
{
    double dx = a.x - b.x;
    double dy = a.y - b.y;
    return std::sqrt(dx*dx + dy*dy);
}

MainWindow::MainWindow(QWidget *parent): QMainWindow(parent), ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    // Графическая сцена и view
    scene = new QGraphicsScene(this);
    ui->graphicsView_map->setScene(scene);
    ui->graphicsView_map->setRenderHint(QPainter::Antialiasing);

    // Установка фильтра событий для ловли кликов мышью по viewport (и зума/пан)
    ui->graphicsView_map->viewport()->installEventFilter(this);

    // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    // ############################### ВСЕ СВЯЗАННОЕ С MENU_BAR ##############################################

    QAction* importJSON = findChild<QAction*>("importJSON");
    QAction* podlozka = findChild<QAction*>("podlozka");
    QAction* dijstra_alg = findChild<QAction*>("dijstra_alg");
    QAction* A_star_alg = findChild<QAction*>("A_star_alg");
    QAction* ant_alg = findChild<QAction*>("ant_alg");
    QAction* Run = findChild<QAction*>("Run");

    connect(importJSON, &QAction::triggered, this, &MainWindow::on_importButton_clicked);
    importJSON->setShortcut(QKeySequence("Ctrl+I"));

    connect(podlozka, &QAction::triggered, this, &MainWindow::on_loadBgButton_clicked);
    podlozka->setShortcut(QKeySequence("Ctrl+B"));

    connect(dijstra_alg, &QAction::triggered, this, &MainWindow::ChoiseDijkstra);

    connect(A_star_alg, &QAction::triggered, this, &MainWindow::ChoiseA_star);

    connect(ant_alg, &QAction::triggered, this, &MainWindow::ChoiseAnt);

    connect(Run, &QAction::triggered, this, &MainWindow::on_buildButton_clicked);
    Run->setShortcut(QKeySequence("Ctrl+R"));

    // #######################################################################################################
    // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

    statusBar()->showMessage("Готов");
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::zoomAt(const QPointF &scenePos, double factor)
{
    // вычислить новый масштаб
    double newScale = currentScale * factor;
    if (newScale < minScale || newScale > maxScale) return;

    // сохранить позицию курсора в сцене до зума
    QPointF beforeCenter = scenePos;

    // выполнить масштабирование
    ui->graphicsView_map->scale(factor, factor);
    currentScale = newScale;

    // после масштабирования отцентрировать так, чтобы точка under cursor осталась под курсором
    // вычисляем смещение в сценовых координатах
    QPointF afterCenter = ui->graphicsView_map->mapToScene(ui->graphicsView_map->viewport()->rect().center());
    QPointF delta = afterCenter - beforeCenter;

    // сдвигаем скроллы чтобы компенсировать
    QScrollBar *h = ui->graphicsView_map->horizontalScrollBar();
    QScrollBar *v = ui->graphicsView_map->verticalScrollBar();
    h->setValue(h->value() + int(delta.x()));
    v->setValue(v->value() + int(delta.y()));
}

// Вот тут придумать как поменять, а то зум постоянно слетает и приходится заново приближать все
void MainWindow::resetViewToFit()
{
    if (!scene->items().isEmpty()) {
        ui->graphicsView_map->fitInView(scene->itemsBoundingRect(), Qt::KeepAspectRatio);
        // обновим currentScale приблизительно (не точно)
        currentScale = 1.0;
    }
}

// (Взято из OpenSource проекта на Git). Почему-то с мышью норм работает, а с тачпадом нет
bool MainWindow::eventFilter(QObject *watched, QEvent *event)
{
    // работаем с viewport у graphicsView_map
    if (watched == ui->graphicsView_map->viewport()) {
        // Колесо — масштаб
        if (event->type() == QEvent::Wheel) {
            QWheelEvent *we = static_cast<QWheelEvent*>(event);
#if QT_VERSION >= QT_VERSION_CHECK(5, 14, 0)
            QPoint numDegrees = we->angleDelta() / 8;
            int delta = numDegrees.y();
#else
            int delta = we->delta();
#endif
            QPoint viewPos = we->position().toPoint();
            QPointF scenePos = ui->graphicsView_map->mapToScene(viewPos);
            if (delta > 0) zoomAt(scenePos, zoomStep);
            else zoomAt(scenePos, 1.0 / zoomStep);
            return true; // событие обработано
        }

        // Нажатие кнопки мыши
        if (event->type() == QEvent::MouseButtonPress) {
            QMouseEvent *me = static_cast<QMouseEvent*>(event);
            if (me->button() == Qt::MiddleButton) {
                // Начать панорамирование
                panning = true;
                lastPanPoint = me->pos();
                ui->graphicsView_map->viewport()->setCursor(Qt::ClosedHandCursor);
                return true;
            }
            if (me->button() == Qt::LeftButton) {
                // оставить обработку выбора вершин как раньше (по левому клику)
                QPoint viewPos = me->pos();
                QPointF scenePos = ui->graphicsView_map->mapToScene(viewPos);
                QList<QGraphicsItem*> itemsAt = scene->items(scenePos);
                for (QGraphicsItem *it : itemsAt) {
                    QGraphicsEllipseItem *ellipse = qgraphicsitem_cast<QGraphicsEllipseItem*>(it);
                    if (ellipse && ellipse->data(0).isValid()) {
                        int idx = ellipse->data(0).toInt();
                        if (startIndex == -1) {
                            startIndex = idx;
                            ellipse->setBrush(QBrush(Qt::blue));
                        } else if (goalIndex == -1 && idx != startIndex) {
                            goalIndex = idx;
                            ellipse->setBrush(QBrush(Qt::red));
                        } else if (idx == startIndex) {
                            ellipse->setBrush(QBrush(Qt::gray));
                            startIndex = -1;
                        } else if (idx == goalIndex) {
                            ellipse->setBrush(QBrush(Qt::gray));
                            goalIndex = -1;
                        } else {
                            clearSelectionColors();
                            startIndex = idx;
                            goalIndex = -1;
                            ellipse->setBrush(QBrush(Qt::blue));
                        }
                        return true;
                    }
                }
            }
        }

        // Перемещение мыши (пан)
        if (event->type() == QEvent::MouseMove) {
            QMouseEvent *me = static_cast<QMouseEvent*>(event);
            if (panning) {
                QPoint delta = me->pos() - lastPanPoint;
                lastPanPoint = me->pos();
                QScrollBar *h = ui->graphicsView_map->horizontalScrollBar();
                QScrollBar *v = ui->graphicsView_map->verticalScrollBar();
                h->setValue(h->value() - delta.x());
                v->setValue(v->value() - delta.y());
                return true;
            }
        }

        // Отпуск кнопки мыши
        if (event->type() == QEvent::MouseButtonRelease) {
            QMouseEvent *me = static_cast<QMouseEvent*>(event);
            if (me->button() == Qt::MiddleButton && panning) {
                panning = false;
                ui->graphicsView_map->viewport()->setCursor(Qt::ArrowCursor);
                return true;
            }
        }
    }

    return QMainWindow::eventFilter(watched, event);
}

// Слот загрузки графа с выбором файла
void MainWindow::on_importButton_clicked()
{
    QString path = QFileDialog::getOpenFileName(this, "Выберите JSON-файл с графом", QString(), "JSON Files (*.json);;All Files (*)");
    if (path.isEmpty())
    {
        statusBar()->showMessage("Файл не указан");
        return;
    }

    QProgressDialog progress("Проверка файла...", QString(), 0, 0, this);

    progress.setWindowModality(Qt::WindowModal);
    progress.setCancelButton(nullptr);
    progress.show();
    pause(5000); // Просто для создания ощущения, что программа "думает"
    qApp->processEvents();

    bool ok = parseJsonFile(path);

    progress.close();

    // [!] Проблема с сбросом подложки при загрузке нового графа. Мб сделать хранение пути к подложке в глобальной переменной и обновлять ее.

    if (ok)
    {
        QMessageBox::information(this, "Импорт", "Успешная загрузка");
        statusBar()->showMessage("Файл загружен");
        drawGraph();
        resetViewToFit(); // вписать граф после загрузки
    } else
    {
        QMessageBox::warning(this, "Импорт", "Некорректный файл");
        statusBar()->showMessage("Некорректный файл");
    }
}

// пауза без юлокировки интерфейса
void MainWindow::pause(int ms)
{
    QEventLoop loop;
    QTimer::singleShot(ms, &loop, &QEventLoop::quit);
    loop.exec();
}

// Слот запуск алгоритма
void MainWindow::on_buildButton_clicked()
{
    if (graph.isEmpty())
    {
        statusBar()->showMessage("Файл не указан");
        return;
    }
    if (startIndex == -1 || goalIndex == -1)
    {
        statusBar()->showMessage("Файл не указан");
        return;
    }

    QVector<int> path;
    if(choise == "A*")
    {
        path = a_star(startIndex, goalIndex);
        statusBar()->showMessage("ready!");
    }
    else if(choise == "Дейкстра")
    {
        path = dijkstra(startIndex, goalIndex);
        statusBar()->showMessage("READY!");
    }
    else
    {
        statusBar()->showMessage("Данный алгоритм временно не доступен");
        return;
    }

    if (path.isEmpty()) {
        statusBar()->showMessage("Нет возможного пути");
        return;
    }

    // Очистить предыдущее выделение и подсветить путь
    clearScene(); // Кстати проблема со сбросом подложки может быть тут    - - - - - - - - - - - - [!]
    drawGraph();
    highlightPath(path);
    statusBar()->setStyleSheet("background-color: rgb(11, 218, 81); color: black;");
    statusBar()->showMessage("Маршрут построен");
    pause(500);
    statusBar()->setStyleSheet("background-color: rgb(255, 255, 255); color: black;");
}

// Парсер JSONa (лучше не трогать, если не изменяется структура). Тут вообще планируется сделать что-то интересное
// по типу выбора нескольких вариантов форматов файлов. Было бы прикольно по фотке чтобы он определял вершины и ребра,
// но как искать тогда веса, вопрос, но можно как вариант оставить Евклидово расстояние. Ну а вообще, из более реалистичного,
// здесь можно подкрутить какой-нибудь OSM как в QGIS
bool MainWindow::parseJsonFile(const QString &path)
{
    QFile f(path);
    if (!f.open(QIODevice::ReadOnly)) return false;
    QByteArray data = f.readAll();
    f.close();

    QJsonParseError jerr;
    QJsonDocument doc = QJsonDocument::fromJson(data, &jerr);
    if (jerr.error != QJsonParseError::NoError) return false;

    QJsonArray arr;
    if (doc.isArray()) {
        arr = doc.array();
    } else if (doc.isObject() && doc.object().contains("vertices") && doc.object().value("vertices").isArray()) {
        arr = doc.object().value("vertices").toArray();
    } else {
        return false;
    }

    QMap<int, Vertex> tmpGraph;
    for (const QJsonValue &v : arr) {
        if (!v.isObject()) return false;
        QJsonObject obj = v.toObject();

        if (!obj.contains("index") || !obj.contains("x") || !obj.contains("y") ||
            !obj.contains("adj") || !obj.contains("weights")) return false;

        Vertex ver;
        ver.index = obj.value("index").toInt();
        ver.x = obj.value("x").toDouble();
        ver.y = obj.value("y").toDouble();
        ver.z = obj.value("z").toDouble();

        QJsonValue adjVal = obj.value("adj");
        QJsonValue wVal = obj.value("weights");
        if (!adjVal.isArray() || !wVal.isArray()) return false;
        QJsonArray adjArr = adjVal.toArray();
        QJsonArray wArr = wVal.toArray();
        if (adjArr.size() != wArr.size()) return false;

        for (int i = 0; i < adjArr.size(); ++i) {
            if (!adjArr[i].isDouble() || !wArr[i].isDouble()) return false;
            ver.adj.append(adjArr[i].toInt());
            ver.weights.append(wArr[i].toDouble());
        }

        tmpGraph.insert(ver.index, ver);
    }

    for (auto it = tmpGraph.constBegin(); it != tmpGraph.constEnd(); ++it) {
        for (int neigh : it.value().adj) {
            if (!tmpGraph.contains(neigh)) return false;
        }
    }

    graph = tmpGraph;
    startIndex = -1;
    goalIndex = -1;
    return true;
}

void MainWindow::drawGraph()
{
    // Очищаем сцену и вспомогательные карты
    //clearScene(); // Еще один вариант, где очищается подложка - - - - - - - - - - - - - - [!]
    edgeItems.clear();
    nodeItems.clear();

    if (graph.isEmpty()) {
        scene->setSceneRect(0,0,100,100);
        return;
    }

    // 1) найдем min/max по исходным координатам (они в lon/lat)
    double minX = std::numeric_limits<double>::infinity();
    double minY = std::numeric_limits<double>::infinity();
    double maxX = -std::numeric_limits<double>::infinity();
    double maxY = -std::numeric_limits<double>::infinity();

    for (auto it = graph.constBegin(); it != graph.constEnd(); ++it) {
        const Vertex &v = it.value();
        if (v.x < minX) minX = v.x;
        if (v.x > maxX) maxX = v.x;
        if (v.y < minY) minY = v.y;
        if (v.y > maxY) maxY = v.y;
    }

    // safety: если все в одной точке, задаём минимальный разброс, а то они
    double width = maxX - minX;
    double height = maxY - minY;
    if (width < 1e-9) width = 1.0;
    if (height < 1e-9) height = 1.0;

    const double targetSize = 2000.0;
    double scale = targetSize / std::max(width, height);

    const double padding = 50.0; // отступы по краям в сценных единицах

    QMap<int, QPointF> sceneCoords; // index -> scene point
    for (auto it = graph.constBegin(); it != graph.constEnd(); ++it)
    {
        const Vertex &v = it.value();
        double sx = (v.x - minX) * scale + padding;
        // flip Y: в гео y увеличивается на север, а в scene — вниз, поэтому инвертируем,
        // чтобы север был вверх на экране
        double sy = (maxY - v.y) * scale + padding;
        sceneCoords.insert(v.index, QPointF(sx, sy));
    }


    for (auto it = graph.constBegin(); it != graph.constEnd(); ++it)
    {
        const Vertex &v = it.value();
        for (int i = 0; i < v.adj.size(); ++i)
        {
            int to = v.adj[i];
            QPair<int,int> key = qMakePair(qMin(v.index, to), qMax(v.index, to));
            if (edgeItems.contains(key)) continue;
            if (!sceneCoords.contains(v.index) || !sceneCoords.contains(to)) continue;
            QPointF p1 = sceneCoords.value(v.index);
            QPointF p2 = sceneCoords.value(to);
            QGraphicsLineItem *line = scene->addLine(p1.x(), p1.y(), p2.x(), p2.y(), QPen(Qt::darkGray, 1));
            line->setZValue(0);
            edgeItems.insert(key, line);
        }
    }

    const double r = 7.0; // радиус вершины (в сценных единицах)
    for (auto it = graph.constBegin(); it != graph.constEnd(); ++it) {
        const Vertex &v = it.value();
        QPointF p = sceneCoords.value(v.index);
        QGraphicsEllipseItem *ellipse = scene->addEllipse(p.x() - r, p.y() - r, r*2, r*2,
                                                          QPen(Qt::black), QBrush(Qt::gray));
        ellipse->setData(0, v.index);
        ellipse->setZValue(1);
        nodeItems.insert(v.index, ellipse);
    }

    scene->setSceneRect(scene->itemsBoundingRect());
    ui->graphicsView_map->fitInView(scene->sceneRect(), Qt::KeepAspectRatio);

    // сброс скейла внутренний, текущий масштаб логически 1
    currentScale = 1.0;
}

// Очистка сцены от графа
void MainWindow::clearScene()
{
    scene->clear();
    edgeItems.clear();
    nodeItems.clear();
}

// Очистка выделенного маршрута
void MainWindow::clearSelectionColors()
{
    if (startIndex != -1 && nodeItems.contains(startIndex))
    {
        nodeItems[startIndex]->setBrush(QBrush(Qt::gray));
    }
    if (goalIndex != -1 && nodeItems.contains(goalIndex))
    {
        nodeItems[goalIndex]->setBrush(QBrush(Qt::gray));
    }
    startIndex = -1;
    goalIndex = -1;
}

// Выделение найденного маршрута цветом
void MainWindow::highlightPath(const QVector<int> &path)
{
    for (int i = 0; i < path.size(); ++i)
    {
        int idx = path[i];
        if (nodeItems.contains(idx))
        {
            if (idx == startIndex) nodeItems[idx]->setBrush(QBrush(Qt::blue));
            else if (idx == goalIndex) nodeItems[idx]->setBrush(QBrush(Qt::red));
            else nodeItems[idx]->setBrush(QBrush(Qt::green));
        }
        if (i + 1 < path.size())
        {
            int a = path[i];
            int b = path[i+1];
            QPair<int,int> key = qMakePair(qMin(a,b), qMax(a,b));
            if (edgeItems.contains(key))
            {
                edgeItems[key]->setPen(QPen(Qt::red, 2));
            }
        }
    }
    if (startIndex != -1 && nodeItems.contains(startIndex)) nodeItems[startIndex]->setBrush(QBrush(Qt::blue));
    if (goalIndex != -1 && nodeItems.contains(goalIndex)) nodeItems[goalIndex]->setBrush(QBrush(Qt::red));
}

// - - - - - - - - - - - - - - - - Алгоритмы - - - - - - - - - - - - - - - -
QVector<int> MainWindow::a_star(int start, int goal)
{
    if (!graph.contains(start) || !graph.contains(goal)) return {};

    const double INF = std::numeric_limits<double>::infinity();
    QMap<int, double> gScore;
    QMap<int, double> fScore;
    QMap<int, int> cameFrom;

    for (auto it = graph.constBegin(); it != graph.constEnd(); ++it)
    {
        gScore[it.key()] = INF;
        fScore[it.key()] = INF;
    }
    gScore[start] = 0.0;
    fScore[start] = heuristicEuclid(graph[start], graph[goal]);

    using Pair = std::pair<double,int>;
    struct Cmp
    {
        bool operator()(const Pair &a, const Pair &b) const
        {
            return a.first > b.first;
        }
    };
    std::priority_queue<Pair, std::vector<Pair>, Cmp> openSet;
    openSet.push({fScore[start], start});
    QSet<int> openSetContain;
    openSetContain.insert(start);

    while (!openSet.empty())
    {
        int current = openSet.top().second;
        openSet.pop();
        openSetContain.remove(current);

        if (current == goal)
        {
            QVector<int> path;
            int cur = goal;
            while (cameFrom.contains(cur))
            {
                path.prepend(cur);
                cur = cameFrom[cur];
            }
            path.prepend(start);
            return path;
        }

        const Vertex &cv = graph[current];
        for (int i = 0; i < cv.adj.size(); ++i)
        {
            int neighbor = cv.adj[i];
            double weight = cv.weights[i];
            double tentative_g = gScore[current] + weight;
            if (tentative_g < gScore[neighbor])
            {
                cameFrom[neighbor] = current;
                gScore[neighbor] = tentative_g;
                fScore[neighbor] = tentative_g + heuristicEuclid(graph[neighbor], graph[goal]);
                if (!openSetContain.contains(neighbor))
                {
                    openSet.push({fScore[neighbor], neighbor});
                    openSetContain.insert(neighbor);
                }
            }
        }
    }
    return {};
}

QVector<int> MainWindow::dijkstra(int start, int goal)
{
    if (!graph.contains(start) || !graph.contains(goal)) return {};

    const double INF = std::numeric_limits<double>::infinity();

    QMap<int, double> dist;
    QMap<int, int> prev;

    for (auto it = graph.constBegin(); it != graph.constEnd(); ++it)
    {
        dist[it.key()] = INF;
    }
    dist[start] = 0.0;

    using Pair = std::pair<double,int>;
    struct Cmp
    {
        bool operator()(const Pair &a, const Pair &b) const
        {
            return a.first > b.first;
        }
    };
    std::priority_queue<Pair, std::vector<Pair>, Cmp> pq;
    pq.push({0.0, start});
    QSet<int> visited;

    while (!pq.empty())
    {
        auto top = pq.top(); pq.pop();
        double d = top.first;
        int u = top.second;

        if (d > dist[u]) continue;
        if (visited.contains(u)) continue;
        visited.insert(u);

        if (u == goal) break;

        const Vertex &vu = graph[u];
        for (int i = 0; i < vu.adj.size(); ++i)
        {
            int v = vu.adj[i];
            double w = vu.weights[i];
            double nd = dist[u] + w;
            if (nd < dist[v])
            {
                dist[v] = nd;
                prev[v] = u;
                pq.push({nd, v});
            }
        }
    }

    if (!prev.contains(goal) && start != goal)
    {
        if (start == goal) return QVector<int>{start};
        return {};
    }

    QVector<int> path;
    int cur = goal;
    path.prepend(cur);
    while (cur != start)
    {
        if (!prev.contains(cur))
        {
            return {};
        }
        cur = prev[cur];
        path.prepend(cur);
    }
    return path;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

//  - - - - - - - - - - - - - - - - Загрузка подложки - - - - - - - - - - - - - - - -
void MainWindow::on_loadBgButton_clicked()
{
    QString path = QFileDialog::getOpenFileName(this, "Выберите изображение подложки", QString(),
                                                "Images (*.png *.jpg *.jpeg *.bmp);;All Files (*)");
    if (path.isEmpty()) return;

    QPixmap pm(path);
    if (pm.isNull())
    {
        QMessageBox::warning(this, "Подложка", "Не удалось загрузить изображение");
        return;
    }

    // Сохраняем оригинал для последующего масштабирования
    bgPixmap = pm;

    if (bgItem)
    {
        scene->removeItem(bgItem);
        delete bgItem;
        bgItem = nullptr;
    }

    // Добавим pixmap в сцену с отрицательным z, чтобы он был под графом
    bgItem = scene->addPixmap(pm);
    bgItem->setZValue(-10);
    bgItem->setOpacity(0.85); // можно регулировать прозрачность
    // Позиция и масштаб будут откорректированы в drawGraph() ниже
    // Если сцена уже содержит элементы, подгоним размер:
    if (!scene->items().isEmpty())
    {
        QRectF bbox = scene->itemsBoundingRect();
        if (bbox.isValid())
        {
            QSize targetSize = bbox.size().toSize();
            if (targetSize.width() > 0 && targetSize.height() > 0)
            {
                QPixmap scaled = bgPixmap.scaled(targetSize, bgKeepAspect ? Qt::KeepAspectRatioByExpanding : Qt::IgnoreAspectRatio,
                                                 Qt::SmoothTransformation);
                bgItem->setPixmap(scaled);
                // Центрируем подложку по bounding rect
                QPointF pos = QPointF(bbox.left(), bbox.top());
                // Если KeepAspectByExpanding — смещаем чтобы покрыть bbox
                if (bgKeepAspect)
                {
                    QSizeF s = QSizeF(scaled.width(), scaled.height());
                    QPointF offset((bbox.width() - s.width())/2.0, (bbox.height() - s.height())/2.0);
                    pos += offset;
                }
                bgItem->setPos(pos);
            }
        }
    }
    // подгоняем вид
    scene->setSceneRect(scene->itemsBoundingRect());
    ui->graphicsView_map->fitInView(scene->sceneRect(), Qt::KeepAspectRatio);
}

// Функции, которые нужны для выбора алгоритма(если алгоритмов будет много, то надо думать что-то другое, а пока что и так сойдет)
void MainWindow::ChoiseDijkstra()
{
    choise = "Дейкстра";
    ui->dijstra_alg->setChecked(true);
    statusBar()->showMessage("Вы выбрали алгоритм Дейкстры");
    pause(1000);
    statusBar()->clearMessage();
    if(ui->A_star_alg->isChecked())
    {
        ui->A_star_alg->setChecked(false);
    }
}

void MainWindow::ChoiseA_star()
{
    choise = "A*";
    ui->A_star_alg->setChecked(true);
    if(ui->dijstra_alg->isChecked())
    {
        ui->dijstra_alg->setChecked(false);
    }
}

void MainWindow::ChoiseAnt()
{
    choise = "Муравьиный поиск";
}


void MainWindow::on_MainWindow_iconSizeChanged(const QSize &iconSize)
{
    ui->centralwidget->size();
}

