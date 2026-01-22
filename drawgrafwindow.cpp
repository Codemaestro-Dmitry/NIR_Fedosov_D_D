#include "drawgrafwindow.h"
#include "ui_drawgrafwindow.h"

#include<QGraphicsScene>
#include<QGraphicsView>
#include<QGraphicsEllipseItem>
#include<QGraphicsLineItem>
#include<QGraphicsPixmapItem>
#include<QGraphicsSimpleTextItem>
#include<QGraphicsSceneMouseEvent>
#include<QFileDialog>
#include<QMessageBox>
#include<QDialog>
#include<QVBoxLayout>
#include<QHBoxLayout>
#include<QLabel>
#include<QLineEdit>
#include<QRadioButton>
#include<QDialogButtonBox>
#include<QPushButton>
#include<QFileInfo>
#include<QFile>
#include<QDir>
#include<QJsonDocument>
#include<QJsonObject>
#include<QJsonArray>
#include<QPainter>
#include<QPen>
#include<QDoubleValidator>
#include<cmath>
#include<QToolButton>
#include<QInputDialog>
#include<QSaveFile>


constexpr double PI_CONST = 3.14159265358979323846;

//////////////////////////////////////////////////////////////////////////
// Вспомогательные графические элементы: NodeItem, EdgeItem, CustomScene
//////////////////////////////////////////////////////////////////////////

class DrawGrafWindow; // forward

class NodeItem : public QGraphicsEllipseItem {
public:
    int id;
    DrawGrafWindow *owner;
    QGraphicsSimpleTextItem *labelItem;

    NodeItem(int id_, const QPointF &pos, DrawGrafWindow *owner_, qreal radius = 12.0)
        : QGraphicsEllipseItem(-radius, -radius, radius*2, radius*2), id(id_), owner(owner_)
    {
        setFlags(QGraphicsItem::ItemIsSelectable | QGraphicsItem::ItemIsMovable | QGraphicsItem::ItemSendsGeometryChanges);
        setPos(pos);
        labelItem = new QGraphicsSimpleTextItem(QString::number(id), this);
        QRectF br = labelItem->boundingRect();
        labelItem->setPos(-br.width()/2, -br.height()/2);
        setBrush(Qt::white);
        setPen(QPen(Qt::black));
    }

    QPointF center() const { return mapToScene(rect().center()); }

protected:
    QVariant itemChange(GraphicsItemChange change, const QVariant &value) override;
};

class EdgeItem : public QGraphicsLineItem {
public:
    int fromId;
    int toId;
    double weightA;
    double weightB;
    bool directed;
    bool bidirectional;

    EdgeItem(int f, int t, double wa, double wb, bool dir, bool bidir, QGraphicsItem *parent = nullptr)
        : QGraphicsLineItem(parent), fromId(f), toId(t),
        weightA(wa), weightB(wb), directed(dir), bidirectional(bidir)
    {
        setZValue(-1);
        setPen(QPen(Qt::darkGray, 2));
    }

    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget = nullptr) override
    {
        Q_UNUSED(option);
        Q_UNUSED(widget);
        painter->setPen(pen());
        painter->drawLine(line());

        // подпись веса(ов)
        QPointF mid = (line().p1() + line().p2()) / 2.0;
        QString wstr;
        if (bidirectional) wstr = QString("%1 / %2").arg(weightA).arg(weightB);
        else wstr = QString::number(weightA);
        painter->save();
        painter->drawText(mid + QPointF(6, -6), wstr);
        painter->restore();

        // стрелка для направленного
        if (directed) {
            const double arrowSize = 8.0;
            QLineF ln(line());
            if (ln.length() > 0.1) {
                double angle = std::atan2(-ln.dy(), ln.dx());
                QPointF p2 = ln.p2();
                QPointF arrowP1 = p2 + QPointF(-arrowSize * std::cos(angle + PI_CONST/6.0),
                                               arrowSize * std::sin(angle + PI_CONST/6.0));
                QPointF arrowP2 = p2 + QPointF(-arrowSize * std::cos(angle - PI_CONST/6.0),
                                               arrowSize * std::sin(angle - PI_CONST/6.0));
                QPolygonF poly;
                poly << p2 << arrowP1 << arrowP2;
                painter->setBrush(pen().color());
                painter->drawPolygon(poly);
            }
        }
    }
};

class CustomScene : public QGraphicsScene {
public:
    explicit CustomScene(QObject *parent = nullptr) : QGraphicsScene(parent), owner(nullptr) {}
    DrawGrafWindow *owner;

protected:
    void mousePressEvent(QGraphicsSceneMouseEvent *event) override {
        if (owner) owner->onSceneMousePress(event);
        QGraphicsScene::mousePressEvent(event);
    }
    void mouseMoveEvent(QGraphicsSceneMouseEvent *event) override {
        if (owner) owner->onSceneMouseMove(event);
        QGraphicsScene::mouseMoveEvent(event);
    }
    void mouseReleaseEvent(QGraphicsSceneMouseEvent *event) override {
        if (owner) owner->onSceneMouseRelease(event);
        QGraphicsScene::mouseReleaseEvent(event);
    }
};

//////////////////////////////////////////////////////////////////////////
// NodeItem::itemChange (реагируем на перемещение)
//////////////////////////////////////////////////////////////////////////

QVariant NodeItem::itemChange(GraphicsItemChange change, const QVariant &value)
{
    if (change == QGraphicsItem::ItemPositionHasChanged && owner) {
        QVariant v = value;
        if (v.canConvert<QPointF>()) {
            QPointF newPos = v.toPointF();
            owner->nodeItemMoved(id, newPos);
        }
    }
    return QGraphicsEllipseItem::itemChange(change, value);
}

//////////////////////////////////////////////////////////////////////////
// DrawGrafWindow - реализация
//////////////////////////////////////////////////////////////////////////

DrawGrafWindow::DrawGrafWindow(QWidget *parent)
    : QDialog(parent),
    ui(new Ui::drawgrafwindow),
    m_scene(nullptr),
    m_tempLine(nullptr),
    m_nextNodeId(1),
    m_mode(Mode_None),
    m_edgeFromId(-1)
{
    ui->setupUi(this);

    // сцена
    CustomScene *cs = new CustomScene(this);
    cs->owner = this;
    m_scene = cs;
    ui->graphicsView->setScene(m_scene);
    ui->graphicsView->setRenderHint(QPainter::Antialiasing);

    // кнопки и сигналы
    connect(ui->loadBgButton, &QPushButton::clicked, this, &DrawGrafWindow::on_loadBgButton_clicked);
    connect(ui->addNodeButton, &QToolButton::toggled, this, &DrawGrafWindow::on_addNodeButton_toggled);
    connect(ui->addEdgeButton, &QToolButton::toggled, this, &DrawGrafWindow::on_addEdgeButton_toggled);
    connect(ui->deleteButton, &QToolButton::toggled, this, &DrawGrafWindow::on_deleteButton_toggled);
    connect(ui->saveButton, &QPushButton::clicked, this, &DrawGrafWindow::on_saveButton_clicked);
    connect(ui->closeButton, &QPushButton::clicked, this, &DrawGrafWindow::on_closeButton_clicked);

    ui->statusLabel->setText("Режим: Обычный");
}

DrawGrafWindow::~DrawGrafWindow()
{
    delete ui;
}

void DrawGrafWindow::on_loadBgButton_clicked()
{
    QString path = QFileDialog::getOpenFileName(this, "Выберите изображение подложки", QString(),
                                                "Images (*.png *.jpg *.jpeg *.bmp);;All Files (*)");
    if (path.isEmpty()) return;

    QPixmap origPix(path);
    if (origPix.isNull()) {
        QMessageBox::warning(this, "Ошибка", "Не удалось загрузить изображение.");
        return;
    }

    // размер области просмотра (viewport) — это пространство, в которое хотим вписать фон
    QSize viewSize = ui->graphicsView->viewport()->size();
    // оставим небольшой отступ, чтобы элементы не упирались в края
    const int margin = 8;
    QSize targetSize = viewSize - QSize(margin, margin);
    if (targetSize.width() <= 0 || targetSize.height() <= 0) targetSize = viewSize;

    // масштабируем оригинал так, чтобы он влез в viewport сохраняя соотношение сторон
    QPixmap scaledPix = origPix.scaled(targetSize, Qt::KeepAspectRatio, Qt::SmoothTransformation);

    // удалим предыдущий фон, если был
    for (QGraphicsItem *it : m_scene->items()) {
        if (auto *p = dynamic_cast<QGraphicsPixmapItem*>(it)) {
            if (p->data(0).toString() == "bg") {
                m_scene->removeItem(p);
                delete p;
            }
        }
    }

    // добавим в сцену уже масштабированный pixmap
    QGraphicsPixmapItem *bg = m_scene->addPixmap(scaledPix);
    bg->setZValue(-100);
    bg->setData(0, "bg");
    m_scene->setSceneRect(bg->boundingRect());

    // Сброс трансформации view и вписывание сцены в view (на случай, если были трансформы)
    ui->graphicsView->resetTransform();
    ui->graphicsView->fitInView(bg, Qt::KeepAspectRatio);

    // сохраняем путь к оригиналу (для копирования при сохранении)
    m_bgPath = path;

    ui->statusLabel->setText(QString("Фон: %1").arg(QFileInfo(m_bgPath).fileName()));
}

void DrawGrafWindow::on_addNodeButton_toggled(bool checked)
{
    if (checked) {
        m_mode = Mode_AddNode;
        m_edgeFromId = -1;
        ui->statusLabel->setText("Режим: Добавление вершины");
        ui->addEdgeButton->setChecked(false);
        ui->deleteButton->setChecked(false);
    } else {
        m_mode = Mode_None;
        ui->statusLabel->setText("Режим: Обычный");
    }
}

void DrawGrafWindow::on_addEdgeButton_toggled(bool checked)
{
    if (checked) {
        m_mode = Mode_AddEdge;
        m_edgeFromId = -1;
        ui->statusLabel->setText("Режим: Добавление ребра (нажать и тянуть)");
        ui->addNodeButton->setChecked(false);
        ui->deleteButton->setChecked(false);
    } else {
        m_mode = Mode_None;
        m_edgeFromId = -1;
        ui->statusLabel->setText("Режим: Обычный");
    }
}

void DrawGrafWindow::on_deleteButton_toggled(bool checked)
{
    if (checked) {
        m_mode = Mode_Delete;
        ui->statusLabel->setText("Режим: Удаление");
        ui->addNodeButton->setChecked(false);
        ui->addEdgeButton->setChecked(false);
    } else {
        m_mode = Mode_None;
        ui->statusLabel->setText("Режим: Обычный");
    }
}

void DrawGrafWindow::onSceneMousePress(QGraphicsSceneMouseEvent *event)
{
    QPointF scenePos = event->scenePos();
    if (m_mode == Mode_AddNode) {
        addNodeAt(scenePos);
        return;
    }
    if (m_mode == Mode_AddEdge) {
        int id = nodeIdAt(scenePos);
        if (id >= 0) {
            startEdgeDrag(id, scenePos);
        } else {
            ui->statusLabel->setText("Кликните по вершине и тяните к другой вершине.");
        }
        return;
    }
    if (m_mode == Mode_Delete) {
        QList<QGraphicsItem*> hits = m_scene->items(scenePos);
        for (QGraphicsItem *it : hits) {
            if (NodeItem *n = dynamic_cast<NodeItem*>(it)) {
                int nid = n->id;
                QVector<NodeData> newNodes;
                for (const NodeData &nd : m_nodes) if (nd.id != nid) newNodes.append(nd);
                m_nodes = newNodes;
                QVector<EdgeData> newEdges;
                for (const EdgeData &ed : m_edges) {
                    if (ed.from == nid || ed.to == nid) continue;
                    newEdges.append(ed);
                }
                m_edges = newEdges;
                redrawAll();
                ui->statusLabel->setText(QString("Вершина %1 удалена").arg(nid));
                return;
            }
            if (EdgeItem *e = dynamic_cast<EdgeItem*>(it)) {
                QVector<EdgeData> newEdges;
                for (const EdgeData &ed : m_edges) {
                    if (ed.from == e->fromId && ed.to == e->toId && qFuzzyCompare(ed.weightA + 1.0, e->weightA + 1.0) && ed.directed == e->directed) {
                        // удаляем
                    } else newEdges.append(ed);
                }
                m_edges = newEdges;
                redrawAll();
                ui->statusLabel->setText("Ребро удалено");
                return;
            }
        }
        ui->statusLabel->setText("Нечего удалять здесь");
        return;
    }
    // обычный режим — ничего
}

void DrawGrafWindow::onSceneMouseMove(QGraphicsSceneMouseEvent *event)
{
    if (m_mode == Mode_AddEdge && m_tempLine) updateEdgeDrag(event->scenePos());
}

void DrawGrafWindow::onSceneMouseRelease(QGraphicsSceneMouseEvent *event)
{
    if (m_mode == Mode_AddEdge && m_tempLine) finishEdgeDrag(event->scenePos());
}

void DrawGrafWindow::startEdgeDrag(int nodeId, const QPointF &pos)
{
    m_edgeFromId = nodeId;
    QPointF start = m_idToItem.contains(nodeId) ? m_idToItem[nodeId]->center() : pos;
    m_tempLine = m_scene->addLine(QLineF(start, pos), QPen(Qt::red, 2, Qt::DashLine));
    ui->statusLabel->setText(QString("Начат перенос от вершины %1").arg(nodeId));
}

void DrawGrafWindow::updateEdgeDrag(const QPointF &pos)
{
    if (!m_tempLine) return;
    QLineF ln = m_tempLine->line();
    ln.setP2(pos);
    m_tempLine->setLine(ln);
}

void DrawGrafWindow::finishEdgeDrag(const QPointF &pos)
{
    if (!m_tempLine) return;
    int toId = nodeIdAt(pos);
    int fromId = m_edgeFromId;
    m_edgeFromId = -1;

    m_scene->removeItem(m_tempLine);
    delete m_tempLine;
    m_tempLine = nullptr;

    if (toId < 0 || toId == fromId) {
        ui->statusLabel->setText("Отмена: нужно отпустить на другой вершине.");
        return;
    }

    // диалог создания ребра
    QDialog dlg(this);
    dlg.setWindowTitle("Параметры ребра");
    QVBoxLayout *vl = new QVBoxLayout(&dlg);
    QLabel *lbl = new QLabel(QString("Ребро %1 ↔ %2").arg(fromId).arg(toId));
    vl->addWidget(lbl);

    QRadioButton *rb12 = new QRadioButton(QString("%1 → %2").arg(fromId).arg(toId));
    QRadioButton *rb21 = new QRadioButton(QString("%1 → %2").arg(toId).arg(fromId));
    QRadioButton *rbBoth = new QRadioButton("Двусторонняя");
    rb12->setChecked(true);
    vl->addWidget(rb12);
    vl->addWidget(rb21);
    vl->addWidget(rbBoth);

    QHBoxLayout *hl1 = new QHBoxLayout;
    QLabel *w1l = new QLabel("Вес (из 1):");
    QLineEdit *we1 = new QLineEdit("1");
    we1->setValidator(new QDoubleValidator(&dlg));
    hl1->addWidget(w1l);
    hl1->addWidget(we1);
    vl->addLayout(hl1);

    QHBoxLayout *hl2 = new QHBoxLayout;
    QLabel *w2l = new QLabel("Вес (из 2):");
    QLineEdit *we2 = new QLineEdit("1");
    we2->setValidator(new QDoubleValidator(&dlg));
    hl2->addWidget(w2l);
    hl2->addWidget(we2);
    vl->addLayout(hl2);

    // по умолчанию второй вес выключен
    we2->setEnabled(false);
    w2l->setEnabled(false);
    connect(rbBoth, &QRadioButton::toggled, this, [we2, w2l](bool on){ we2->setEnabled(on); w2l->setEnabled(on); });

    QDialogButtonBox *bb = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    vl->addWidget(bb);
    connect(bb, &QDialogButtonBox::accepted, &dlg, &QDialog::accept);
    connect(bb, &QDialogButtonBox::rejected, &dlg, &QDialog::reject);

    if (dlg.exec() == QDialog::Accepted) {
        double a = we1->text().toDouble();
        double b = we2->text().toDouble();
        if (rb12->isChecked()) {
            createEdge(fromId, toId, a, 0.0, true, false);
            ui->statusLabel->setText(QString("Добавлено ребро %1 → %2 (w=%3)").arg(fromId).arg(toId).arg(a));
        } else if (rb21->isChecked()) {
            createEdge(toId, fromId, a, 0.0, true, false);
            ui->statusLabel->setText(QString("Добавлено ребро %1 → %2 (w=%3)").arg(toId).arg(fromId).arg(a));
        } else {
            createEdge(fromId, toId, a, b, false, true);
            ui->statusLabel->setText(QString("Добавлено двустороннее ребро %1 ↔ %2 (w1=%3, w2=%4)").arg(fromId).arg(toId).arg(a).arg(b));
        }
    } else {
        ui->statusLabel->setText("Создание ребра отменено.");
    }
}

void DrawGrafWindow::addNodeAt(const QPointF &pos)
{
    NodeData nd;
    nd.id = m_nextNodeId++;
    nd.pos = pos;
    nd.label = QString::number(nd.id);
    m_nodes.append(nd);

    NodeItem *ni = new NodeItem(nd.id, nd.pos, this);
    m_scene->addItem(ni);
    m_idToItem[nd.id] = ni;
}

int DrawGrafWindow::nodeIdAt(const QPointF &pos)
{
    const double pickRadius = 12.0;
    QList<QGraphicsItem*> hits = m_scene->items(QRectF(pos - QPointF(pickRadius, pickRadius), QSizeF(pickRadius*2, pickRadius*2)));
    for (QGraphicsItem *it : hits) {
        if (NodeItem *n = dynamic_cast<NodeItem*>(it)) {
            return n->id;
        }
    }
    return -1;
}

void DrawGrafWindow::createEdge(int fromId, int toId, double weightA, double weightB, bool directed, bool bidir)
{
    EdgeData ed{fromId, toId, weightA, weightB, directed, bidir};
    m_edges.append(ed);
    redrawAll();
}

void DrawGrafWindow::redrawAll()
{
    // сохранить фон (если есть)
    QPixmap bgpix;
    bool hadBg = false;
    for (QGraphicsItem *it : m_scene->items()) {
        if (auto *p = dynamic_cast<QGraphicsPixmapItem*>(it)) {
            if (p->data(0).toString() == "bg") { bgpix = p->pixmap(); hadBg = true; break; }
        }
    }

    m_scene->clear();
    m_idToItem.clear();

    if (hadBg) {
        QGraphicsPixmapItem *bg = m_scene->addPixmap(bgpix);
        bg->setZValue(-100);
        bg->setData(0, "bg");
        m_scene->setSceneRect(bg->boundingRect());
    }

    for (const NodeData &nd : m_nodes) {
        NodeItem *ni = new NodeItem(nd.id, nd.pos, this);
        m_scene->addItem(ni);
        m_idToItem[nd.id] = ni;
    }

    for (const EdgeData &ed : m_edges) {
        NodeItem *na = m_idToItem.value(ed.from, nullptr);
        NodeItem *nb = m_idToItem.value(ed.to, nullptr);
        if (!na || !nb) continue;
        EdgeItem *ei = new EdgeItem(ed.from, ed.to, ed.weightA, ed.weightB, ed.directed, ed.bidirectional);
        ei->setLine(QLineF(na->center(), nb->center()));
        m_scene->addItem(ei);
    }
}

void DrawGrafWindow::nodeItemMoved(int id, const QPointF &newPos)
{
    for (NodeData &nd : m_nodes) {
        if (nd.id == id) { nd.pos = newPos; break; }
    }
    // обновляем линии, которые на сцене
    for (QGraphicsItem *it : m_scene->items()) {
        if (EdgeItem *ei = dynamic_cast<EdgeItem*>(it)) {
            if (ei->fromId == id || ei->toId == id) {
                NodeItem *na = m_idToItem.value(ei->fromId, nullptr);
                NodeItem *nb = m_idToItem.value(ei->toId, nullptr);
                if (na && nb) ei->setLine(QLineF(na->center(), nb->center()));
            }
        }
    }
}

bool DrawGrafWindow::saveJsonToFolder(const QString &folder)
{
    QDir dir(folder);
    if (!dir.exists()) {
        if (!dir.mkpath(".")) return false;
    }

    // Найдём bgItem (если есть) и загрузим оригинал (если есть путь m_bgPath)
    QGraphicsPixmapItem *bg = nullptr;
    for (QGraphicsItem *it : m_scene->items()) {
        if (auto *p = dynamic_cast<QGraphicsPixmapItem*>(it)) {
            if (p->data(0).toString() == QStringLiteral("bg")) { bg = p; break; }
        }
    }

    QPixmap origPix;
    if (!m_bgPath.isEmpty()) origPix = QPixmap(m_bgPath);

    QJsonArray nodesArr;
    for (const NodeData &nd : m_nodes) {
        QJsonObject o;
        o["index"] = nd.id;
        o["z"] = 0.0;

        if (bg && !origPix.isNull()) {
            // получим область фонового изображения на сцене и реальные размеры оригинала
            QRectF br = bg->sceneBoundingRect();
            QSize imgSize = origPix.size();

            double img_x = 0.0, img_y = 0.0;
            if (br.width() > 0 && br.height() > 0 && imgSize.width() > 0 && imgSize.height() > 0) {
                double relx = (nd.pos.x() - br.left()) / br.width();
                double rely = (nd.pos.y() - br.top())  / br.height();
                img_x = relx * imgSize.width();
                img_y = rely * imgSize.height();
            }

            o["img_x"] = img_x;
            o["img_y"] = img_y;

            // сохранение сценных координат как запасной вариант
            o["x_scene"] = nd.pos.x();
            o["y_scene"] = nd.pos.y();
        } else if (bg) {
            // есть bgItem, но не удалось загрузить оригинал — сохраняем относительные координаты
            QRectF br = bg->sceneBoundingRect();
            double relx = 0.0, rely = 0.0;
            if (br.width() > 0 && br.height() > 0) {
                relx = (nd.pos.x() - br.left()) / br.width();
                rely = (nd.pos.y() - br.top())  / br.height();
            }
            o["rel_x"] = relx;
            o["rel_y"] = rely;
            o["x_scene"] = nd.pos.x();
            o["y_scene"] = nd.pos.y();
        } else {
            // фон отсутствует — сохраняем сценные координаты
            o["x_scene"] = nd.pos.x();
            o["y_scene"] = nd.pos.y();
        }

        // собираем списки соседей и весов для этой вершины
        QJsonArray adjArr;
        QJsonArray wArr;
        for (const EdgeData &ed : m_edges) {
            if (ed.from == nd.id) {
                adjArr.append(ed.to);
                wArr.append(ed.weightA);
            } else if (ed.to == nd.id && ed.bidirectional) {
                // если двустороннее ребро — добавить обратную связь с weightB
                adjArr.append(ed.from);
                wArr.append(ed.weightB);
            }
        }
        o["adj"] = adjArr;
        o["weights"] = wArr;

        nodesArr.append(o);
    }

    QJsonObject root;
    // coord_type: предпочтительно сохранять в пикселях оригинала, если доступен
    if (bg && !origPix.isNull()) root["coord_type"] = QStringLiteral("image_pixels");
    else if (bg) root["coord_type"] = QStringLiteral("relative_bg");
    else root["coord_type"] = QStringLiteral("scene");

    // имя фонового файла (без пути) — мы копируем фон в папку при сохранении, так mainwindow сможет его найти
    if (!m_bgPath.isEmpty()) root["bg_file"] = QFileInfo(m_bgPath).fileName();
    root["nodes"] = nodesArr;

    QJsonDocument doc(root);

    // атомарно записываем map.json
    QSaveFile sf(dir.filePath(QStringLiteral("map.json")));
    if (!sf.open(QIODevice::WriteOnly)) {
        return false;
    }
    sf.write(doc.toJson(QJsonDocument::Indented));
    if (!sf.commit()) {
        return false;
    }

    return true;
}


bool DrawGrafWindow::copyBackgroundToFolder(const QString &folder)
{
    if (m_bgPath.isEmpty()) return true;
    QFileInfo fi(m_bgPath);
    QDir dir(folder);
    QString target = dir.filePath(fi.fileName());
    if (QFile::exists(target)) QFile::remove(target);
    return QFile::copy(m_bgPath, target);
}

void DrawGrafWindow::on_saveButton_clicked()
{
    QString parent = QFileDialog::getExistingDirectory(this, "Выберите папку для сохранения");
    if (parent.isEmpty()) return;
    bool ok = false;
    QString name = QInputDialog::getText(this, "Имя папки", "Введите имя папки для сохранения карты:", QLineEdit::Normal, "my_map", &ok);
    if (!ok || name.isEmpty()) return;
    QDir dir(parent);
    QString full = dir.filePath(name);
    QDir m(full);
    if (!m.exists()) {
        if (!dir.mkdir(name)) {
            QMessageBox::warning(this, "Ошибка", "Не удалось создать папку: " + full);
            return;
        }
    }

    if (!saveJsonToFolder(full)) {
        QMessageBox::warning(this, "Ошибка", "Не удалось сохранить JSON.");
        return;
    }
    if (!copyBackgroundToFolder(full)) {
        QMessageBox::warning(this, "Предупреждение", "Не удалось скопировать фон (но JSON сохранён).");
    }
    QMessageBox::information(this, "Сохранено", "Карта сохранена: " + full);
    emit graphSaved(full);
}

// Сбрасываем и применяем светлую тему
void DrawGrafWindow::applyLightTheme()
{
    // Сбросим предыдущий stylesheet (чтобы не было перекрытий)
    this->setStyleSheet(QString());

    // Применяем чистый светлый стиль
    this->setStyleSheet(R"(
QWidget {
    background-color: #ffffff;
    color: #222222;
    font-family: "Segoe UI", "Roboto", "Arial";
    font-size: 12px;
}

/* Центральные области */
QWidget#centralwidget, QFrame#centralwidget {
    background-color: #f8f9fa;
    border: 1px solid #e0e0e0;
    border-radius: 6px;
}

/* GraphicsView */
QGraphicsView {
    background-color: #ffffff;
    border: 1px solid #dcdcdc;
}

/* Кнопки */
QPushButton {
    background: qlineargradient(x1:0,y1:0,x2:1,y2:1, stop:0 #ffffff, stop:1 #f0f0f0);
    border: 1px solid #d0d0d0;
    color: #222222;
    padding: 6px 10px;
    border-radius: 6px;
}
QPushButton:hover { background: #f2f6fa; }
QPushButton:pressed { background: #e6eef9; }

/* ToolButton (иконки, режимы) */
QToolButton {
    color: #222222;
    background: transparent;
}
QToolButton:hover { background: rgba(0,0,0,0.03); }

/* Поля ввода */
QLineEdit, QTextEdit, QPlainTextEdit {
    background-color: #ffffff;
    border: 1px solid #e0e0e0;
    color: #222222;
    border-radius: 4px;
    padding: 4px;
}

/* Диалоги */
QDialog {
    background-color: #ffffff;
}

/* Label */
QLabel { color: #222222; }

/* Статус */
QStatusBar { background: #f5f5f5; color: #333333; }

/* Tooltip */
QToolTip {
    background-color: #ffffff;
    color: #222222;
    border: 1px solid #cccccc;
}
    )");
}

// Тёмная тема — вариант тот, что ты уже использовал (можно оставить)
void DrawGrafWindow::applyDarkTheme()
{
    this->setStyleSheet(QString()); // очистка
    this->setStyleSheet(R"(
QWidget {
    background-color: qlineargradient(x1:0,y1:0,x2:1,y2:1,
        stop:0 rgba(18,22,30,220), stop:1 rgba(10,12,18,230));
    color: rgba(240,246,255,0.95);
    font-family: "Segoe UI", "Roboto", "Arial";
    font-size: 12px;
}
QWidget#centralwidget, QFrame#centralwidget {
    background-color: rgba(255,255,255,0.03);
    border: 1px solid rgba(255,255,255,0.06);
    border-radius: 10px;
}
QGraphicsView { background-color: rgba(255,255,255,0.02); border: 1px solid rgba(200,210,255,0.06); }
QPushButton { color: rgba(240,246,255,0.95); background: qlineargradient(x1:0,y1:0,x2:1,y2:1, stop:0 rgba(130,180,255,0.12), stop:1 rgba(200,160,255,0.10)); border: 1px solid rgba(255,255,255,0.07); padding: 6px 12px; border-radius: 8px; }
QToolButton { color: rgba(240,246,255,0.95); }
QLineEdit, QTextEdit, QPlainTextEdit { color: rgba(240,246,255,0.95); background-color: rgba(255,255,255,0.02); border: 1px solid rgba(255,255,255,0.05); border-radius: 6px; padding: 6px; }
QDialog { background-color: rgba(14,18,24,240); }
QLabel { color: rgba(240,246,255,0.95); }
QStatusBar { color: rgba(200,210,230,0.9); }
    )");
}


void DrawGrafWindow::on_closeButton_clicked()
{
    close();
}
