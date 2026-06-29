#include "BaseItem.h"
#include <math.h>
#include <QtDebug>
#include <QThread>

#define M_PI 3.14159265358979323846
qreal BaseItem::ContrSize = 8;

void BaseItem::SetIndex(int num)
{
	if (ControlList.length() > 0)
	{
		ControlList[0]->index = num;
	}
}

void BaseItem::SetScale(double value)
{

}

BaseItem::BaseItem(QPointF center, ItemType type) :center(center), types(type)
{
	setHandlesChildEvents(false);//设置后才能将事件传递到子元素
	noSelected.setColor(QColor(0, 100, 200));
	noSelected.setWidth(LineWidth);
	isSelected.setColor(QColor(0, 180, 0));
	isSelected.setWidth(LineWidth);
	fillColor = QColor(0, 160, 230, 50); //填充颜色
	thisPen = noSelected;
	if (type != AutoItem)       //模式选择 自定义模式用于显示亚像素轮廓和Region  不设定任何属性
	{
		this->setFlags(QGraphicsItem::ItemIsSelectable |
			QGraphicsItem::ItemIsMovable |
			QGraphicsItem::ItemIsFocusable);
	}
}

void BaseItem::focusInEvent(QFocusEvent* event)
{
	Q_UNUSED(event);
	thisPen = isSelected;
	for (int i = 1; i < ControlList.length(); i++)
	{
		ControlList[i]->setVisible(true);
	}
}

void BaseItem::focusOutEvent(QFocusEvent* event)
{
	Q_UNUSED(event);
	thisPen = noSelected;
	for (int i = 1; i < ControlList.length(); i++)
	{
		ControlList[i]->setVisible(false);
	}
}

void BaseItem::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget)
{
	//缩放控制点尺寸
	for (int i = 0; i < ControlList.size(); i++)
	{
		ControlList[i]->setScale(1 / (*scale));
	}	
	painter->save();
	painter->setBrush(Qt::NoBrush);
	QFont font;
	font.setPointSizeF(15 / (*scale)); //字体大小
	painter->setFont(font);
	painter->setPen(Qt::black);
	painter->drawText(ControlList[0]->GetPoint() + QPointF(-ContrSize / (*scale), -(ContrSize + 3) / (*scale)), ItemDiscrib);
	painter->restore();
	//子类绘制时状态
	LineWidth = 2 / (*scale);
	thisPen.setWidthF(LineWidth);
	painter->setPen(thisPen);
}



//******绘制结果数据******
DrawItem::DrawItem() :BaseItem(QPointF(0, 0), AutoItem)
{
	returnRec = QRectF(0, 0, 10000, 10000);
}

void DrawItem::Refresh()
{
	this->update();
}

QRectF DrawItem::boundingRect() const
{
	//当不设定Rect时超出区域会发生不显示的情况
	return returnRec;
}

void DrawItem::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget)
{
	Q_UNUSED(option);
	Q_UNUSED(widget);
	LineWidth = 2 / (*scale);
	DrawPen.setWidthF(LineWidth);
	DrawPen.setColor(QColor(255, 0, 0));
	painter->setPen(DrawPen);
	for (int i = 0; i < LineList.size(); i++)
	{
		if (LineList[i].size() <= 1)
		{
		}
		else
		{
			for (int j = 0; j < LineList[i].size() - 1; j++)
			{
				painter->drawLine(LineList[i][j], LineList[i][j - 1]);
			}
		}
	}
	for (auto item : ROIList)
	{
		for (auto p : item)
		{
			painter->drawPoint(p);
		}
	}
}



#pragma region SLOT形
qreal vectorLength(const QPointF& vector)
{
	return std::sqrt(vector.x() * vector.x() + vector.y() * vector.y());
}

//内左上，右上，左下，右下。 外同
QVector<QPointF> calculateMatrixCorners(const QVector<QPointF>& points)
{
	// 确保输入的点的数量正确
	if (points.size() != 4) {
		qDebug() << "Invalid input: Expected 4 points (3 inner matrix corners + 1 outer top-left corner).";
		return QVector<QPointF>(); // 返回空的 QVector
	}

	// 解析输入的内侧矩阵的三个点和外侧矩阵的左上角点
	QPointF innerTopLeft = points[0];
	QPointF innerTopRight = points[1];
	QPointF outerTopLeft = points[3]; // 外侧矩阵的左上角点

	// 计算内侧矩阵的边向量
	QPointF innerEdgeX = innerTopRight - innerTopLeft;  // 右上角到左上角的向量
	float dx = points[1].x() - points[0].x();
	float dy = points[1].y() - points[0].y();
	float num = (dx * (points[2].y() - points[0].y())) - (dy * (points[2].x() - points[0].x()));
	float den = dx * dx + dy * dy;
	den = sqrt(den);
	QPointF normal(-dy, dx);
	normal = normal / den;
	QPointF innerEdgeY = (num / den) * normal;
	QPointF innerBottomLeft = innerTopLeft + innerEdgeY;
	QPointF innerBottomRight = innerTopRight + innerEdgeY;

	// 计算内侧矩阵的实际宽度和高度
	qreal innerWidth = vectorLength(innerEdgeX);
	qreal innerHeight = vectorLength(innerEdgeY);

	QPointF v40(points[3].x() - points[0].x(), points[3].y() - points[0].y());
	qreal dis_v40_x = abs(QPointF::dotProduct(v40, innerEdgeX) / innerWidth);
	qreal dis_v40_y = abs(QPointF::dotProduct(v40, innerEdgeY) / innerHeight);

	// 根据给定公式计算外侧矩阵的边长
	qreal outerWidth = dis_v40_x * 2 + innerWidth;
	qreal outerHeight = dis_v40_y * 2 + innerHeight;

	// 根据外侧矩阵的边长重新计算方向向量
	QPointF outerEdgeX = innerEdgeX * (outerWidth / innerWidth); // 水平方向
	QPointF outerEdgeY = innerEdgeY * (outerHeight / innerHeight); // 垂直方向



	// 计算外侧矩阵的其它三个点
	QPointF outerTopRight = outerTopLeft + outerEdgeX;
	QPointF outerBottomLeft = outerTopLeft + outerEdgeY;
	QPointF outerBottomRight = outerTopRight + outerEdgeY;

	// 将所有点添加到返回的 QVector 中
	QVector<QPointF> result;
	result.append(innerTopLeft);
	result.append(innerTopRight);
	result.append(innerBottomLeft);
	result.append(innerBottomRight);
	result.append(outerTopLeft);
	result.append(outerTopRight);
	result.append(outerBottomLeft);
	result.append(outerBottomRight);

	return result;
}

//构造四个控制点，内圈左上点，内圈右上点，内圈下线任意点，外圈左上点
SlotItem::SlotItem(qreal x, qreal y, qreal width, qreal height, qreal thickness) :BaseItem(QPointF(0, 0), SLOT)
{
	control_points.push_back(QPointF(x, y));
	control_points.push_back(QPointF(x + width, y+100));
	control_points.push_back(QPointF(x, y + height));
	control_points.push_back(QPointF(x - thickness, y - thickness));
	eight_points = calculateMatrixCorners(control_points);

	center = (eight_points[0] + eight_points[3]) / 2;
	ControlList << new ControlItem(this, center, 0);
	ControlList << new ControlItem(this, control_points[0], 1);
	ControlList << new ControlItem(this, control_points[1], 2);
	ControlList << new ControlItem(this, control_points[2], 3);
	ControlList << new ControlItem(this, control_points[3], 4);
}

//四个关键点，内圈左上，内圈右上，内圈下，外圈左上
SlotItem::SlotItem(QPointF ilt, QPointF irt, QPointF id, QPointF olt) :BaseItem(QPointF(0,0), SLOT)
{
	control_points.push_back(ilt);
	control_points.push_back(irt);
	control_points.push_back(id);
	control_points.push_back(olt);
	eight_points = calculateMatrixCorners(control_points);

	center = (eight_points[0] + eight_points[3]) / 2;

	ControlList << new ControlItem(this, center, 0);
	ControlList << new ControlItem(this, control_points[0], 1);
	ControlList << new ControlItem(this, control_points[1], 2);
	ControlList << new ControlItem(this, control_points[2], 3);
	ControlList << new ControlItem(this, control_points[3], 4);
}
bool SlotItem::UpDate(int index)
{
	control_points[index-1] = ControlList[index]->GetPoint();
	eight_points = calculateMatrixCorners(control_points);
	center = (eight_points[0] + eight_points[3]) / 2;
	return true;
}




void SlotItem::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget)
{
	QThread::msleep(10);
	Q_UNUSED(option);
	Q_UNUSED(widget);
	BaseItem::paint(painter, option, widget);

	painter->setRenderHint(QPainter::Antialiasing);



	// 设置绘制的颜色和宽度
	QPen darkBluePen;
	darkBluePen.setColor(Qt::darkBlue);
	darkBluePen.setWidth(2); // 设置线条宽度
	painter->setPen(darkBluePen);

	painter->setBrush(QBrush(fillColor));

	QPainterPath path;
	path.moveTo(eight_points[2]);
	path.lineTo(eight_points[3]);

	// 计算直线的方向，得到直线的角度
	QLineF line(eight_points[0], eight_points[1]);
	qreal angle = line.angle();
	qreal radius = QLineF(eight_points[0], eight_points[2]).length()/2;

	{
		QRectF rect((eight_points[1].x() + eight_points[3].x()) / 2 - radius, (eight_points[1].y() + eight_points[3].y()) / 2 - radius, radius * 2, radius * 2);
		path.arcTo(rect, angle - 90, 180);
	}

	path.moveTo(eight_points[1]);
	path.lineTo(eight_points[0]);
	{
		QRectF rect((eight_points[0].x() + eight_points[2].x()) / 2 - radius, (eight_points[0].y() + eight_points[2].y()) / 2 - radius, radius * 2, radius * 2);
		path.arcTo(rect, angle + 90, 180);
	}

	radius = QLineF(eight_points[4], eight_points[6]).length() / 2;
	path.moveTo(eight_points[6]);
	path.lineTo(eight_points[7]);
	{
		QRectF rect((eight_points[5].x() + eight_points[7].x()) / 2 - radius, (eight_points[5].y() + eight_points[7].y()) / 2 - radius, radius * 2, radius * 2);
		path.arcTo(rect, angle - 90, 180);
	}
	path.moveTo(eight_points[5]);
	path.lineTo(eight_points[4]);
	{
		QRectF rect((eight_points[4].x() + eight_points[6].x()) / 2 - radius, (eight_points[4].y() + eight_points[6].y()) / 2 - radius, radius * 2, radius * 2);
		path.arcTo(rect, angle + 90, 180);
	}

	painter->drawPath(path);
	//{
	//	// 获取矩形的宽度和高度
	//	qreal width = std::sqrt(std::pow(control_points[0].x() - control_points[1].x(), 2) + std::pow(control_points[0].y() - control_points[1].y(), 2));
	//	qreal height = std::sqrt(std::pow(control_points[0].x() - control_points[2].x(), 2) + std::pow(control_points[0].y() - control_points[2].y(), 2));
	//	// 绘制左侧半圆
	//	qreal radius = height / 2.0;
	//	QPointF leftCenter = (eight_points[0] + eight_points[2]) / 2;
	//	QRectF leftCircleRect(leftCenter.x() - radius, leftCenter.y() - radius, 2 * radius, 2 * radius);
	//	painter->drawArc(leftCircleRect, 90 * 16, 180 * 16);  // 绘制左侧半圆


	//	// 绘制右侧半圆
	//	QPointF rightCenter = (eight_points[1] + eight_points[3]) / 2;
	//	QRectF rightCircleRect(rightCenter.x() - radius, rightCenter.y() - radius, 2 * radius, 2 * radius);
	//	painter->drawArc(rightCircleRect, -90 * 16, 180 * 16);  // 绘制右侧半圆

	//	// 绘制上边的直线
	//	painter->drawLine(eight_points[0], eight_points[1]);
	//	painter->drawLine(eight_points[2], eight_points[3]);
	//}

	//{
	//	// 获取矩形的宽度和高度
	//	qreal width = std::sqrt(std::pow(eight_points[4].x() - eight_points[5].x(), 2) + std::pow(eight_points[4].y() - eight_points[5].y(), 2));
	//	qreal height = std::sqrt(std::pow(eight_points[4].x() - eight_points[6].x(), 2) + std::pow(eight_points[4].y() - eight_points[6].y(), 2));
	//	// 绘制左侧半圆
	//	qreal radius = height / 2.0;
	//	QPointF leftCenter = (eight_points[4] + eight_points[6]) / 2;
	//	QRectF leftCircleRect(leftCenter.x() - radius, leftCenter.y() - radius, 2 * radius, 2 * radius);
	//	painter->drawArc(leftCircleRect, 90 * 16, 180 * 16);  // 绘制左侧半圆


	//	// 绘制右侧半圆
	//	QPointF rightCenter = (eight_points[5] + eight_points[7]) / 2;
	//	QRectF rightCircleRect(rightCenter.x() - radius, rightCenter.y() - radius, 2 * radius, 2 * radius);
	//	painter->drawArc(rightCircleRect, -90 * 16, 180 * 16);  // 绘制右侧半圆

	//	// 绘制上边的直线
	//	painter->drawLine(eight_points[4], eight_points[5]);
	//	painter->drawLine(eight_points[6], eight_points[7]);
	//}

}

QRectF SlotItem::boundingRect() const
{
	qreal dis4c = vectorLength(eight_points[4] - center);
	return QRectF(center.x()- dis4c, center.y() - dis4c, dis4c*2, dis4c*2);
}

#pragma endregion

#pragma region 矩形
RectangleItem::RectangleItem(qreal x, qreal y, qreal width, qreal height) :BaseItem(QPointF(x, y), Rectangle)
{
	ControlList << new ControlItem(this, QPointF(x + width / 2, y + height / 2), 0);
	ControlList << new ControlItem(this, QPointF(x, y), 1);
	ControlList << new ControlItem(this, QPointF(x + width, y), 2);
	ControlList << new ControlItem(this, QPointF(x + width, y + height), 3);
	ControlList << new ControlItem(this, QPointF(x, y + height), 4);
}

QRectF RectangleItem::boundingRect() const
{
	return  QRectF(ControlList[1]->GetPoint(), ControlList[3]->GetPoint());
}

bool RectangleItem::UpDate(int index)
{
	QPointF Pf = ControlList[index]->GetPoint();
	//角点分情况
	switch (index)
	{
	case 1:
		ControlList[2]->SetPoint(QPointF(ControlList[2]->GetPoint().x(), Pf.y()));
		ControlList[4]->SetPoint(QPointF(Pf.x(), ControlList[4]->GetPoint().y()));
		break;
	case 2:
		ControlList[1]->SetPoint(QPointF(ControlList[1]->GetPoint().x(), Pf.y()));
		ControlList[3]->SetPoint(QPointF(Pf.x(), ControlList[3]->GetPoint().y()));
		break;
	case 3:
		ControlList[2]->SetPoint(QPointF(Pf.x(), ControlList[2]->GetPoint().y()));
		ControlList[4]->SetPoint(QPointF(ControlList[4]->GetPoint().x(), Pf.y()));
		break;
	case 4:
		ControlList[1]->SetPoint(QPointF(Pf.x(), ControlList[1]->GetPoint().y()));
		ControlList[3]->SetPoint(QPointF(ControlList[3]->GetPoint().x(), Pf.y()));
		break;
	default:
		break;
	}
	//中心点
	ControlList[0]->SetPoint(QPointF((ControlList[1]->GetPoint().x() + ControlList[2]->GetPoint().x()) / 2, (ControlList[2]->GetPoint().y() + ControlList[3]->GetPoint().y()) / 2));
	return true;
}

void RectangleItem::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget)
{
	QThread::msleep(10);
	Q_UNUSED(option);
	Q_UNUSED(widget);
	BaseItem::paint(painter, option, widget);
	QRectF ret(ControlList[1]->GetPoint(), ControlList[3]->GetPoint());
	painter->drawRect(ret);
}

void RectangleItem::SetRect(MRectangle MRect)
{
	QPointF set = this->mapFromScene(MRect.col, MRect.row);
	ControlList[0]->SetPoint(set + QPointF(MRect.width / 2, MRect.height / 2));
	ControlList[1]->SetPoint(set);
	ControlList[2]->SetPoint(set + QPointF(MRect.width, 0));
	ControlList[3]->SetPoint(set + QPointF(MRect.width, MRect.height));
	ControlList[4]->SetPoint(set + QPointF(0, MRect.height));
	this->setFocus();
}

void RectangleItem::GetRect(MRectangle& MRect)
{
	QPointF GetRec = this->mapToScene(ControlList[1]->x(), ControlList[1]->y());
	MRect.col = GetRec.x();
	MRect.row = GetRec.y();
	MRect.width = ControlList[2]->GetPoint().x() - ControlList[1]->GetPoint().x();
	MRect.height = ControlList[3]->GetPoint().y() - ControlList[1]->GetPoint().y();
}
#pragma endregion

#pragma region 旋转矩形
RectangleRItem::RectangleRItem(qreal x, qreal y, qreal Lenth1, qreal Lenth2, qreal Pi) :BaseItem(QPointF(x, y), RectangleR)
{
	angle = Pi;
	lenth1 = Lenth1 / 2;
	lenth2 = Lenth2 / 2;
	qreal s = sin(-angle);
	qreal c = cos(-angle);
	Pa1 = center + QPointF(0 * c - lenth2 * s, 0 * s + lenth2 * c);
	Pa2 = center + QPointF(lenth1 * c - 0 * s, lenth1 * s + 0 * c);
	Pa3 = center + QPointF(0 * c + lenth2 * s, 0 * s - lenth2 * c);
	Pa4 = center + QPointF(-lenth1 * c - 0 * s, -lenth1 * s + 0 * c);
	PArrow = center + QPointF((lenth1 + 20) * c - 0 * s, (lenth1 + 20) * s + 0 * c);
	//中心
	ControlList << new ControlItem(this, center, 0);
	//中心线控制点
	ControlList << new ControlItem(this, Pa1, 1);
	ControlList << new ControlItem(this, Pa2, 2);
	ControlList << new ControlItem(this, Pa3, 3);
	ControlList << new ControlItem(this, Pa4, 4);
}

QRectF RectangleRItem::boundingRect() const
{
	qreal tmp = (lenth1 > lenth2 ? lenth2 : lenth1);
	return QRectF(center.x() - tmp, center.y() - tmp, tmp * 2, tmp * 2);
}

bool RectangleRItem::UpDate(int index)
{
	QPointF Pf = ControlList[index]->GetPoint();
	qreal dx = Pf.x() - center.x();
	qreal dy = Pf.y() - center.y();
	if (dx >= 0 && dy < 0)
	{
		angle = atan2((-1) * (dy), dx);
	}
	else if (dx < 0 && dy < 0)
	{
		angle = atan2((-1) * dy, dx);
	}
	else if (dx < 0 && dy >= 0)
	{
		angle = M_PI * 2 + atan2((-1) * dy, dx);
	}
	else if (dx >= 0 && dy >= 0)
	{
		angle = M_PI * 2 - atan2(dy, dx);
	}
	//角度补偿 角度方向逆时针 控制点排列方向 顺时针
	switch (index)
	{
	case 1:
		angle += M_PI / 2;
		break;
	case 3:
		angle += M_PI * 3 / 2;
		break;
	case 4:
		angle += M_PI;
		break;
	}
	qreal s = sin(-angle);
	qreal c = cos(-angle);
	if (index == 2 || index == 4)
	{
		lenth1 = sqrt(dx * dx + dy * dy);
	}
	else if (index == 1 || index == 3)
	{
		lenth2 = sqrt(dx * dx + dy * dy);
	}
	Pa1 = center + QPointF(0 * c - lenth2 * s, 0 * s + lenth2 * c);
	Pa2 = center + QPointF(lenth1 * c - 0 * s, lenth1 * s + 0 * c);
	Pa3 = center + QPointF(0 * c + lenth2 * s, 0 * s - lenth2 * c);
	Pa4 = center + QPointF(-lenth1 * c - 0 * s, -lenth1 * s + 0 * c);
	PArrow = center + QPointF((lenth1 + 20) * c - 0 * s, (lenth1 + 20) * s + 0 * c);
	ControlList[1]->SetPoint(Pa1);
	ControlList[2]->SetPoint(Pa2);
	ControlList[3]->SetPoint(Pa3);
	ControlList[4]->SetPoint(Pa4);
	return true;
}

void RectangleRItem::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget)
{
	QThread::msleep(10);
	Q_UNUSED(option);
	Q_UNUSED(widget);
	BaseItem::paint(painter, option, widget);
	//画角度方向线
	painter->drawLine(PArrow, Pa2);
	//绘制方向箭头
	float l = 15.0;//箭头长度
	double atn1 = atan2((PArrow.y() - Pa2.y()), (PArrow.x() - Pa2.x()));
	double atn2 = atan2((PArrow.x() - Pa2.x()), (PArrow.y() - Pa2.y()));
	QPointF Arrow1(PArrow.x() - l * cos(atn1 - 0.5), PArrow.y() - l * sin(atn1 - 0.5));
	QPointF Arrow2(PArrow.x() - l * sin(atn2 - 0.5), PArrow.y() - l * cos(atn2 - 0.5));
	painter->drawLine(PArrow, Arrow1);
	painter->drawLine(PArrow, Arrow2);
	//绘制旋转矩形
	painter->save();
	painter->translate(center);
	painter->rotate(-angle * 180 / M_PI);
	painter->drawRect(QRectF(-lenth1, -lenth2, lenth1 * 2, lenth2 * 2));
	painter->restore();
}

void RectangleRItem::GetRotatedRect(MRotatedRect& MRRect)
{
	QPointF GetRRect = this->mapToScene(ControlList[0]->x(), ControlList[0]->y());
	MRRect.col = GetRRect.x();
	MRRect.row = GetRRect.y();
	MRRect.phi = angle;
	MRRect.lenth1 = lenth1;
	MRRect.lenth2 = lenth2;
}
#pragma endregion

#pragma region 多边形
PolygonItem::PolygonItem() :BaseItem(QPointF(0, 0), Polygon)
{
	init_points.clear();
	list_ps.clear();
	ControlList << new ControlItem(this, center, 0);
	ControlList[0]->setVisible(false);
	Finished = false;
}

QPointF PolygonItem::getCentroid(QList<QPointF> list)
{
	qreal x = 0;
	qreal y = 0;
	for (auto& temp : list)
	{
		x += temp.x();
		y += temp.y();
	}
	x = x / list.size();
	y = y / list.size();
	return QPointF(x, y);
}

void PolygonItem::getMaxLength()
{
	QVector<qreal> vec;
	vec.reserve(200);
	vec.clear();
	for (int i = 1; i < ControlList.length(); i++)
	{
		qreal dis = sqrt(pow(center.x() - ControlList[i]->x(), 2) + pow(center.y() - ControlList[i]->y(), 2));
		vec.append(dis);
	}
	qreal ret = 0;
	for (auto& temp : vec)
	{
		if (temp > ret) {
			ret = temp;
		}
	}
	Radius = ret;
}

void PolygonItem::pushPoint(QPointF p, QList<QPointF> list, bool isCenter)
{
	if (!Finished)
	{
		center = getCentroid(list);
		getMaxLength();
		if (isCenter)
		{
			ControlList[0]->SetPoint(center);
			ControlList[0]->setVisible(true);
			Finished = true;
		}
		else
		{
			auto tmp = new ControlItem(this, p, ControlList.length());
			tmp->setVisible(true);
			ControlList << tmp;
			init_points.append(p);
			list_ps.append(list);
		}		
		this->update();
	}
}

QRectF PolygonItem::boundingRect() const
{
	return QRectF(center.x() - Radius, center.y() - Radius, Radius * 2, Radius * 2);
}

bool PolygonItem::UpDate(int index)
{
	Q_UNUSED(index);
	QList<QPointF> list;
	list.reserve(200);
	list.clear();
	for (int i = 1; i < ControlList.length(); i++)
	{
		list << ControlList[i]->GetPoint();
	}
	center = getCentroid(list);
	ControlList[0]->SetPoint(center);
	return true;
}

void PolygonItem::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget)
{
	QThread::msleep(10);
	Q_UNUSED(option);
	Q_UNUSED(widget);
	thisPen.setWidthF(LineWidth);
	painter->setPen(thisPen);
	if (Finished)
	{
		for (int i = 1; i < ControlList.length() - 1; i++)
		{
			painter->drawLine(ControlList[i]->GetPoint(), ControlList[i + 1]->GetPoint());
		}
		painter->drawLine(ControlList[ControlList.length() - 1]->GetPoint(), ControlList[1]->GetPoint());
	}
	else
	{
		for (int i = 1; i < ControlList.length() - 1; i++)
		{
			painter->drawLine(ControlList[i]->GetPoint(), ControlList[i + 1]->GetPoint());
		}
	}
}

void PolygonItem::GetPolygon(MPolygon& mpolygon)
{
	QList<QPointF> list_p;
	list_p.reserve(200);
	list_p.clear();
	for (int i = 1; i < ControlList.length(); i++)
	{
		list_p.append(mapToScene(ControlList[i]->GetPoint()));
	}
	mpolygon.list_p = list_p;
	mpolygon.points = init_points;
	mpolygon.list_ps = list_ps;
}
#pragma endregion

#pragma region 椭圆
EllipseItem::EllipseItem(qreal x, qreal y, qreal Lenth1, qreal Lenth2, qreal Pi) :RectangleRItem(x, y, Lenth1, Lenth2, Pi)
{
}

void EllipseItem::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget)
{
	QThread::msleep(10);
	Q_UNUSED(option);
	Q_UNUSED(widget);
	BaseItem::paint(painter, option, widget);
	painter->drawLine(center, Pa2);
	painter->save();
	painter->translate(center);
	painter->rotate(-angle * 180 / M_PI);
	painter->drawEllipse(QRectF(-lenth1, -lenth2, lenth1 * 2, lenth2 * 2));
	painter->restore();
}
#pragma endregion

#pragma region 圆
CircleItem::CircleItem(qreal x, qreal y, qreal R) :BaseItem(QPointF(x, y), Circle)
{
	Radius = R;
	ControlList << new ControlItem(this, center, 0);
	ControlList << new ControlItem(this, center + QPointF(R, 0), 1);
}

QRectF CircleItem::boundingRect() const
{
	return QRectF(center.x() - Radius, center.y() - Radius, Radius * 2, Radius * 2);
}

bool CircleItem::UpDate(int index)
{
	QPointF Pf = ControlList[index]->GetPoint();
	QPointF tmp = Pf - center;
	Radius = sqrt(tmp.x() * tmp.x() + tmp.y() * tmp.y());
	return true;
}

void CircleItem::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget)
{
	QThread::msleep(10);
	Q_UNUSED(option);
	Q_UNUSED(widget);
	BaseItem::paint(painter, option, widget);
	painter->drawEllipse(QRectF(center.x() - Radius, center.y() - Radius, Radius * 2, Radius * 2));
}

void CircleItem::GetCircle(MCircle& Cir)
{
	QPointF GetRec = this->mapToScene(ControlList[0]->x(), ControlList[0]->y());
	Cir.col = GetRec.x();
	Cir.row = GetRec.y();
	Cir.radius = Radius;
}
#pragma endregion

#pragma region 同心圆
ConcentricCircleItem::ConcentricCircleItem(qreal x, qreal y, qreal radiusMin, qreal radiusMax) :BaseItem(QPointF(x, y), Concentric_Circle)
{
	RadiusMax = radiusMax;
	RadiusMin = radiusMin > radiusMax ? radiusMax : radiusMin;
	ControlList << new ControlItem(this, center, 0);
	ControlList << new ControlItem(this, center + QPointF(RadiusMin, 0), 1);
	ControlList << new ControlItem(this, center + QPointF(RadiusMax, 0), 2);
}

QRectF ConcentricCircleItem::boundingRect() const
{
	return QRectF(center.x() - RadiusMax, center.y() - RadiusMax, RadiusMax * 2, RadiusMax * 2);
}

bool ConcentricCircleItem::UpDate(int index)
{
	QPointF Pf = ControlList[index]->GetPoint();
	QPointF tmp = Pf - center;
	qreal R = sqrt(tmp.x() * tmp.x() + tmp.y() * tmp.y());
	if (index == 1)
	{
		if (R > RadiusMax)
			return false;
		RadiusMin = R;
	}
	else if (index == 2)
	{
		if (R < RadiusMin)
			return false;
		RadiusMax = R;
	}
	return true;
}

void ConcentricCircleItem::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget)
{
	QThread::msleep(10);
	Q_UNUSED(option);
	Q_UNUSED(widget);
	BaseItem::paint(painter, option, widget);
	QPainterPath pth;
	pth.addEllipse(QRectF(center.x() - RadiusMin, center.y() - RadiusMin, RadiusMin * 2, RadiusMin * 2));
	pth.addEllipse(QRectF(center.x() - RadiusMax, center.y() - RadiusMax, RadiusMax * 2, RadiusMax * 2));
	painter->setBrush(QBrush(fillColor));
	painter->drawPath(pth);
	//绘制分割线	
	painter->setPen(QPen(QColor(10, 255, 255, 255), 1));
	QLine line;
	double angle = 0;
	ring_small_points.clear();
	ring_big_points.clear();
	for (int i = 0; i <= segment_line_num; i++)
	{
		QPointF pf_min, pf_max;
		if (0 <= angle && angle < 90)
		{
			line = QLine(QPoint(center.x() + cos(angle * M_PI / 180) * RadiusMin, center.y() - sin(angle * M_PI / 180) * RadiusMin), QPoint(center.x() + cos(angle * M_PI / 180) * RadiusMax, center.y() - sin(angle * M_PI / 180) * RadiusMax));
			pf_min = this->mapToScene(QPointF(center.x() + cos(angle * M_PI / 180) * RadiusMin, center.y() - sin(angle * M_PI / 180) * RadiusMin));
			ring_small_points.push_back(pf_min);
			pf_max = this->mapToScene(QPointF(center.x() + cos(angle * M_PI / 180) * RadiusMax, center.y() - sin(angle * M_PI / 180) * RadiusMax));
			ring_big_points.push_back(pf_max);
		}
		else if (90 <= angle && angle < 180)
		{
			line = QLine(QPoint(center.x() - sin((angle - 90) * M_PI / 180) * RadiusMin, center.y() - cos((angle - 90) * M_PI / 180) * RadiusMin), QPoint(center.x() - sin((angle - 90) * M_PI / 180) * RadiusMax, center.y() - cos((angle - 90) * M_PI / 180) * RadiusMax));
			pf_min = this->mapToScene(QPointF(center.x() - sin((angle - 90) * M_PI / 180) * RadiusMin, center.y() - cos((angle - 90) * M_PI / 180) * RadiusMin));
			ring_small_points.push_back(pf_min);
			pf_max = this->mapToScene(QPointF(center.x() - sin((angle - 90) * M_PI / 180) * RadiusMax, center.y() - cos((angle - 90) * M_PI / 180) * RadiusMax));
			ring_big_points.push_back(pf_max);
		}
		else if (180 <= angle && angle < 270)
		{
			line = QLine(QPoint(center.x() - cos((angle - 180) * M_PI / 180) * RadiusMin, center.y() + sin((angle - 180) * M_PI / 180) * RadiusMin), QPoint(center.x() - cos((angle - 180) * M_PI / 180) * RadiusMax, center.y() + sin((angle - 180) * M_PI / 180) * RadiusMax));
			pf_min = this->mapToScene(QPointF(center.x() - cos((angle - 180) * M_PI / 180) * RadiusMin, center.y() + sin((angle - 180) * M_PI / 180) * RadiusMin));
			ring_small_points.push_back(pf_min);
			pf_max = this->mapToScene(QPointF(center.x() - cos((angle - 180) * M_PI / 180) * RadiusMax, center.y() + sin((angle - 180) * M_PI / 180) * RadiusMax));
			ring_big_points.push_back(pf_max);
		}
		else if (270 <= angle && angle < 360)
		{
			line = QLine(QPoint(center.x() + sin((angle - 270) * M_PI / 180) * RadiusMin, center.y() + cos((angle - 270) * M_PI / 180) * RadiusMin), QPoint(center.x() + sin((angle - 270) * M_PI / 180) * RadiusMax, center.y() + cos((angle - 270) * M_PI / 180) * RadiusMax));
			pf_min = this->mapToScene(QPointF(center.x() + sin((angle - 270) * M_PI / 180) * RadiusMin, center.y() + cos((angle - 270) * M_PI / 180) * RadiusMin));
			ring_small_points.push_back(pf_min);
			pf_max = this->mapToScene(QPointF(center.x() + sin((angle - 270) * M_PI / 180) * RadiusMax, center.y() + cos((angle - 270) * M_PI / 180) * RadiusMax));
			ring_big_points.push_back(pf_max);
		}
		painter->drawLine(line);
		angle = angle + (360.0 / (double)segment_line_num);
	}
}

void ConcentricCircleItem::GetConcentricCircle(CCircle& CCir)
{
	QPointF GetCCircle = this->mapToScene(ControlList[0]->x(), ControlList[0]->y());
	CCir.col = GetCCircle.x();
	CCir.row = GetCCircle.y();
	CCir.small_radius = RadiusMin;
	CCir.big_radius = RadiusMax;
}

QImage ConcentricCircleItem::Mat2QImage(const cv::Mat& mat)
{
	if (mat.empty())
	{
		return QImage();
	}
	if (mat.type() == CV_8UC1)
	{
		QImage image(mat.cols, mat.rows, QImage::Format_Indexed8);
		image.setColorCount(256);
		for (int i = 0; i < 256; i++)
		{
			image.setColor(i, qRgb(i, i, i));
		}
		uchar* pSrc = mat.data;
		for (int row = 0; row < mat.rows; row++)
		{
			uchar* pDest = image.scanLine(row);
			memcpy(pDest, pSrc, mat.cols);
			pSrc += mat.step;
		}
		return image;
	}
	else if (mat.type() == CV_8UC3)
	{
		const uchar* pSrc = (const uchar*)mat.data;
		QImage image(pSrc, mat.cols, mat.rows, mat.step, QImage::Format_RGB888);
		return image.rgbSwapped();
	}
	else if (mat.type() == CV_8UC4)
	{
		const uchar* pSrc = (const uchar*)mat.data;
		QImage image(pSrc, mat.cols, mat.rows, mat.step, QImage::Format_ARGB32);
		return image.copy();
	}
	else
	{
		return QImage();
	}
}

//寻找边界点
//threshold_delta为阈值；direction的值为0时代表“从白到黑”寻找边界点，direction的值为1时代表“从黑到白”寻找边界点
//in_out_circle值为0时，从内往外扫描，值为1时，从外往内扫描；segment_num为分割线数量
int ConcentricCircleItem::FindBorderPoints(const cv::Mat& src_mat, std::vector<QPointF>& xy)
{
	try
	{
		cv::Mat gray;
		if (src_mat.channels() == 3)
		{
			cv::cvtColor(src_mat, gray, cv::COLOR_BGR2GRAY);
		}
		else if (src_mat.channels() == 4)
		{
			cv::cvtColor(src_mat, gray, cv::COLOR_RGBA2GRAY);
		}
		else
		{
			src_mat.copyTo(gray);
		}
		int ww = gray.cols;
		int hh = gray.rows;
		int bytes = ww * hh;
		std::vector<unsigned char> rgbValues(bytes);
		QPixmap pixmap = QPixmap::fromImage(Mat2QImage(gray));
		QImage img = pixmap.toImage();
		unsigned char* pData = img.bits();
		int i, j;
		for (i = 0; i < ww * hh * 4; i += 4)
		{
			rgbValues[i / 4] = pData[i];
		}
		std::vector<unsigned char> buffer8(bytes);
		buffer8 = rgbValues;
		//边界点                   
		std::vector<QPointF> backPoint(segment_line_num);
		std::vector<QPointF> backOutPoint(segment_line_num);
		//寻找边界点		
		int m = 0;
		//计算两点间的距离
		double distance;
		distance = pow((ring_small_points[0].x() - ring_big_points[0].x()), 2) + pow((ring_small_points[0].y() - ring_big_points[0].y()), 2);
		distance = sqrt(distance);
		//线图像上每一个点灰度颜色
		QList<float> temparrclor = QList<float>();
		temparrclor.reserve(5000);
		//线图像上每一个点坐标
		QList<QPointF> position = QList<QPointF>();
		position.reserve(5000);
		for (int k = 0; k < segment_line_num; k++)
		{
			temparrclor.clear();
			position.clear();
			float x_point;
			float y_point;
			if (in_out_circle == 0)
			{
				for (int n = 0; n < (int)distance; n++)
				{
					x_point = (n * (ring_big_points[k].x() - ring_small_points[k].x())) / distance + ring_small_points[k].x();
					y_point = (n * (ring_big_points[k].y() - ring_small_points[k].y())) / distance + ring_small_points[k].y();
					float tempf = abs(y_point - (int)y_point);
					QPointF tempPt = QPointF(x_point, y_point);
					float avgGrey = (float)buffer8[(int)y_point * ww + (int)x_point];
					float avgGrey1 = (float)buffer8[(int)(y_point + 1) * ww + (int)x_point];
					float grey = avgGrey * (1 - tempf) + avgGrey1 * tempf;
					//插值灰度
					temparrclor.push_back(grey);
					position.push_back(tempPt);
				}
			}
			else if (in_out_circle == 1)
			{
				for (int n = 0; n < (int)distance; n++)
				{
					x_point = (n * (ring_small_points[k].x() - ring_big_points[k].x())) / distance + ring_big_points[k].x();
					y_point = (n * (ring_small_points[k].y() - ring_big_points[k].y())) / distance + ring_big_points[k].y();
					float tempf = abs(y_point - (int)y_point);
					QPointF tempPt = QPointF(x_point, y_point);
					float avgGrey = (float)buffer8[(int)y_point * ww + (int)x_point];
					float avgGrey1 = (float)buffer8[(int)(y_point + 1) * ww + (int)x_point];
					float grey = avgGrey * (1 - tempf) + avgGrey1 * tempf;
					//插值灰度
					temparrclor.push_back(grey);
					position.push_back(tempPt);
				}
			}
			//找穿越点
			QPointF return_point = FindCrosspointimprove(temparrclor, position, direction, threshold_delta);
			if (return_point != QPointF(0, 0))
			{
				backPoint[m] = return_point;
				m += 1;
			}
		}
		backOutPoint.resize(m);
		for (int n = 0; n < m; n++)
		{
			backOutPoint[n] = backPoint[n];
		}
		xy = backOutPoint;
		return 0;
	}
	catch (...)
	{
		return -1;
	}
}

//梯度和求穿越点
QPointF ConcentricCircleItem::FindCrosspointimprove(const QList<float>& lineTiDu, const QList<QPointF>& ijRecord, const int direction, const int threshold_delta)
{
	try
	{
		//梯度和求穿越点，使用五个一组
		QList<float> deltatemp = QList<float>();
		deltatemp.reserve(5000);
		deltatemp.clear();
		QList<float> fenzuaverage = QList<float>();
		fenzuaverage.reserve(5000);
		fenzuaverage.clear();
		for (int i = 0; i < lineTiDu.size() - 1; i++)
		{
			float grey1 = lineTiDu[i + 1];
			float delta = (lineTiDu[i] - grey1);
			if (abs(delta) < 5)
				delta = 0;
			deltatemp.push_back(delta);
		}
		int fenzuC = deltatemp.size() / 5;
		for (int i = 0; i < fenzuC * 5; i += 5)
		{
			float sum5 = deltatemp[i] + deltatemp[i + 1] + deltatemp[i + 2] + deltatemp[i + 3] + deltatemp[i + 4];
			float aver = sum5 / 5;
			if (abs(aver) < 5)
				aver = 0;
			fenzuaverage.push_back(aver);
		}
		if (fenzuaverage.size() == 0) return QPointF();
		int crosspoint = 0;
		int crosspoint1 = 0;
		//从白到黑寻找
		if (direction == 0)
		{
			for (int iii = fenzuaverage.size() - 1; iii >= 0; iii--) //排除起点和终点
			{
				if (fenzuaverage[iii] > 0)
				{
					crosspoint = 5 * (iii);
					break;
				}
			}
			float maxVar = deltatemp[crosspoint];
			for (int m = -5; m < 5; m++)
			{
				int tempint = crosspoint + m;
				if (crosspoint == 0)
					tempint = 0;
				if (deltatemp[tempint] >= maxVar)
				{
					maxVar = deltatemp[tempint];
					crosspoint1 = tempint;
				}
			}
		}
		//从黑到白寻找
		else if (direction == 1)
		{
			for (int iii = 0; iii <= fenzuaverage.size() - 1; iii++) //排除起点和终点
			{
				if (fenzuaverage[iii] < 0)
				{
					crosspoint = 5 * (iii);
					break;
				}
			}
			float minVar = deltatemp[crosspoint];
			for (int m = -5; m < 5; m++)
			{
				int tempint = crosspoint + m;
				if (crosspoint == 0)
					tempint = 0;
				if (deltatemp[tempint] <= minVar)
				{
					minVar = deltatemp[tempint];
					crosspoint1 = tempint;
				}
			}
		}
		//阈值判别
		if (fenzuaverage[crosspoint / 5] * 5 >= threshold_delta || fenzuaverage[crosspoint / 5] * 5 <= -threshold_delta)
		{
			//穿越点的index对应坐标(crosspoint1为线图像中穿越点index)
			return ijRecord[crosspoint1];
		}
		else
		{
			return QPointF(0, 0); //没有穿越点，返回（0,0）
		}
	}
	catch (...)
	{
		return QPointF(0, 0); //没有穿越点，返回（0,0）
	}
}

cv::Vec3f ConcentricCircleItem::fitCircle(const std::vector<cv::Point2f>& points)
{
	if (points.size() < 3)
	{
		return cv::Vec3f();
	}
	int i = 0;
	double X1 = 0;
	double Y1 = 0;
	double X2 = 0;
	double Y2 = 0;
	double X3 = 0;
	double Y3 = 0;
	double X1Y1 = 0;
	double X1Y2 = 0;
	double X2Y1 = 0;
	int Sum = points.size();
	for (i = 0; i < Sum; i++)
	{
		X1 = X1 + points[i].x;
		Y1 = Y1 + points[i].y;
		X2 = X2 + points[i].x * points[i].x;
		Y2 = Y2 + points[i].y * points[i].y;
		X3 = X3 + points[i].x * points[i].x * points[i].x;
		Y3 = Y3 + points[i].y * points[i].y * points[i].y;
		X1Y1 = X1Y1 + points[i].x * points[i].y;
		X1Y2 = X1Y2 + points[i].x * points[i].y * points[i].y;
		X2Y1 = X2Y1 + points[i].x * points[i].x * points[i].y;
	}
	double C, D, E, G, H, N;
	double a, b, c;
	N = points.size();
	C = N * X2 - X1 * X1;
	D = N * X1Y1 - X1 * Y1;
	E = N * X3 + N * X1Y2 - (X2 + Y2) * X1;
	G = N * Y2 - Y1 * Y1;
	H = N * X2Y1 + N * Y3 - (X2 + Y2) * Y1;
	a = (H * D - E * G) / (C * G - D * D);
	b = (H * C - E * D) / (D * D - G * C);
	c = -(a * X1 + b * Y1 + X2 + Y2) / N;
	double A, B, R;
	A = a / (-2);
	B = b / (-2);
	R = sqrt(a * a + b * b - 4 * c) / 2;
	return cv::Vec3f(A, B, R);
}


#pragma endregion

#pragma region 直线
LineItem::LineItem(qreal x1, qreal y1, qreal x2, qreal y2) :BaseItem(QPointF((x1 + x2) / 2, (y1 + y2) / 2), LineObj)
{
	ControlList << new ControlItem(this, center, 0);
	ControlList << new ControlItem(this, QPointF(x1, y1), 1);
	ControlList << new ControlItem(this, QPointF(x2, y2), 2);
	P1 = QPointF(x1, y1);
	P2 = QPointF(x2, y2);
}

bool LineItem::UpDate(int index)
{
	if (index == 1)
	{
		P1 = ControlList[1]->GetPoint();
	}
	else if (index == 2)
	{
		P2 = ControlList[2]->GetPoint();
	}
	ControlList[0]->SetPoint(QPointF((P1.x() + P2.x()) / 2, (P1.y() + P2.y()) / 2));
	return true;
}

void LineItem::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget)
{
	QThread::msleep(10);
	Q_UNUSED(option);
	Q_UNUSED(widget);
	BaseItem::paint(painter, option, widget);
	painter->drawLine(P1, P2);
}
#pragma endregion

#pragma region 卡尺
Caliper::Caliper(qreal x1, qreal y1, qreal x2, qreal y2, qreal height) :BaseItem(QPointF((x1 + x2) / 2, (y1 + y2) / 2), LineObj)
{	
	P1 = QPointF(x1, y1);
	P2 = QPointF(x2, y2);
	Height = height;
	ControlList << new ControlItem(this, center, 0);
	ControlList << new ControlItem(this, QPointF(x1, y1), 1);
	ControlList << new ControlItem(this, QPointF(x2, y2), 2);
	qreal dx = P1.x() - P2.x();
	qreal dy = P1.y() - P2.y();
	if (dx >= 0 && dy < 0)
	{
		angle = atan2((-1) * (dy), dx);
	}
	else if (dx < 0 && dy < 0)
	{
		angle = atan2((-1) * dy, dx);
	}
	else if (dx < 0 && dy >= 0)
	{
		angle = M_PI * 2 + atan2((-1) * dy, dx);
	}
	else if (dx >= 0 && dy >= 0)
	{
		angle = M_PI * 2 - atan2(dy, dx);
	}
	Lenth = sqrt(dx * dx + dy * dy);
	qreal s = sin(angle);
	qreal c = cos(angle);
	ControlList << new ControlItem(this, center + QPointF(s * Height / 2, c * Height / 2), 3);			
}

QRectF Caliper::boundingRect() const
{
	int min = Lenth < Height ? Lenth : Height;
	int size = min < 5 ? 5 : min;
	QPointF c = (P1 + P2) / 2;
	return QRectF(c.x() - size, c.y() - size, size, size);
}

bool Caliper::UpDate(int index)
{
	if (index == 1)
	{
		P1 = ControlList[1]->GetPoint();
	}
	else if (index == 2)
	{
		P2 = ControlList[2]->GetPoint();
	}
	else if (index == 3)
	{
		QPointF Pf = ControlList[3]->GetPoint();
		qreal dx = Pf.x() - center.x();
		qreal dy = Pf.y() - center.y();
		Height = sqrt(dx * dx + dy * dy) * 2;
	}
	center = QPointF((P1.x() + P2.x()) / 2, (P1.y() + P2.y()) / 2);
	ControlList[0]->SetPoint(center);
	qreal dx = P1.x() - P2.x();
	qreal dy = P1.y() - P2.y();
	if (dx >= 0 && dy < 0)
	{
		angle = atan2((-1) * (dy), dx);
	}
	else if (dx < 0 && dy < 0)
	{
		angle = atan2((-1) * dy, dx);
	}
	else if (dx < 0 && dy >= 0)
	{
		angle = M_PI * 2 + atan2((-1) * dy, dx);
	}
	else if (dx >= 0 && dy >= 0)
	{
		angle = M_PI * 2 - atan2(dy, dx);
	}
	qreal s = sin(angle);
	qreal c = cos(angle);
	ControlList[3]->SetPoint(center + QPointF(s * Height / 2, c * Height / 2));
	Lenth = sqrt(dx * dx + dy * dy);
	return true;
}

void Caliper::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget)
{
	QThread::msleep(10);
	Q_UNUSED(option);
	Q_UNUSED(widget);
	BaseItem::paint(painter, option, widget);	
	//绘制分割线	
	QLine line;
	painter->setPen(QPen(QColor(10, 255, 255, 255), 1));
	double angle_buf = 180 * angle / M_PI;
	line_small_points.clear();
	line_big_points.clear();
	for (int i = 0; i <= segment_line_num; i++)
	{
		QPointF pf_min, pf_max;
		if (0 <= angle_buf && angle_buf < 90)
		{
			QPoint RecPS, RecPE, Ps, Pe;
			RecPS = QPoint(P1.x() - 0.5 * (double)Height * sin(angle), P1.y() - 0.5 * (double)Height * cos(angle));
			RecPE = QPoint(P2.x() - 0.5 * (double)Height * sin(angle), P2.y() - 0.5 * (double)Height * cos(angle));
			Ps = QPoint(RecPS.x() + ((double)Height / (double)segment_line_num) * i * sin(angle), RecPS.y() + ((double)Height / (double)segment_line_num) * i * cos(angle));
			Pe = QPoint(RecPE.x() + ((double)Height / (double)segment_line_num) * i * sin(angle), RecPE.y() + ((double)Height / (double)segment_line_num) * i * cos(angle));
			line = QLine(Ps, Pe);
			pf_min = this->mapToScene(QPointF(Ps.x(), Ps.y()));
			line_small_points.push_back(pf_min);
			pf_max = this->mapToScene(QPointF(Pe.x(), Pe.y()));
			line_big_points.push_back(pf_max);
		}
		else if (90 <= angle_buf && angle_buf < 180)
		{
			QPoint RecPS, RecPE, Ps, Pe;
			RecPS = QPoint(P1.x() + 0.5 * (double)Height * cos((angle_buf - 90) * M_PI / 180), P1.y() - 0.5 * (double)Height * sin((angle_buf - 90) * M_PI / 180));
			RecPE = QPoint(P2.x() + 0.5 * (double)Height * cos((angle_buf - 90) * M_PI / 180), P2.y() - 0.5 * (double)Height * sin((angle_buf - 90) * M_PI / 180));
			Ps = QPoint(RecPS.x() - ((double)Height / (double)segment_line_num) * i * cos((angle_buf - 90) * M_PI / 180), RecPS.y() + ((double)Height / (double)segment_line_num) * i * sin((angle_buf - 90) * M_PI / 180));
			Pe = QPoint(RecPE.x() - ((double)Height / (double)segment_line_num) * i * cos((angle_buf - 90) * M_PI / 180), RecPE.y() + ((double)Height / (double)segment_line_num) * i * sin((angle_buf - 90) * M_PI / 180));
			line = QLine(Ps, Pe);
			pf_min = this->mapToScene(QPointF(Ps.x(), Ps.y()));
			line_small_points.push_back(pf_min);
			pf_max = this->mapToScene(QPointF(Pe.x(), Pe.y()));
			line_big_points.push_back(pf_max);
		}
		else if (180 <= angle_buf && angle_buf < 270)
		{
			QPoint RecPS, RecPE, Ps, Pe;
			RecPS = QPoint(P1.x() - 0.5 * (double)Height * sin((angle_buf - 180) * M_PI / 180), P1.y() - 0.5 * (double)Height * cos((angle_buf - 180) * M_PI / 180));
			RecPE = QPoint(P2.x() - 0.5 * (double)Height * sin((angle_buf - 180) * M_PI / 180), P2.y() - 0.5 * (double)Height * cos((angle_buf - 180) * M_PI / 180));
			Ps = QPoint(RecPS.x() + ((double)Height / (double)segment_line_num) * i * sin((angle_buf - 180) * M_PI / 180), RecPS.y() + ((double)Height / (double)segment_line_num) * i * cos((angle_buf - 180) * M_PI / 180));
			Pe = QPoint(RecPE.x() + ((double)Height / (double)segment_line_num) * i * sin((angle_buf - 180) * M_PI / 180), RecPE.y() + ((double)Height / (double)segment_line_num) * i * cos((angle_buf - 180) * M_PI / 180));
			line = QLine(Ps, Pe);
			pf_min = this->mapToScene(QPointF(Ps.x(), Ps.y()));
			line_small_points.push_back(pf_min);
			pf_max = this->mapToScene(QPointF(Pe.x(), Pe.y()));
			line_big_points.push_back(pf_max);
		}
		else if (270 <= angle_buf && angle_buf < 360)
		{
			QPoint RecPS, RecPE, Ps, Pe;
			RecPS = QPoint(P1.x() - 0.5 * (double)Height * cos((angle_buf - 270) * M_PI / 180), P1.y() + 0.5 * (double)Height * sin((angle_buf - 270) * M_PI / 180));
			RecPE = QPoint(P2.x() - 0.5 * (double)Height * cos((angle_buf - 270) * M_PI / 180), P2.y() + 0.5 * (double)Height * sin((angle_buf - 270) * M_PI / 180));
			Ps = QPoint(RecPS.x() + ((double)Height / (double)segment_line_num) * i * cos((angle_buf - 270) * M_PI / 180), RecPS.y() - ((double)Height / (double)segment_line_num) * i * sin((angle_buf - 270) * M_PI / 180));
			Pe = QPoint(RecPE.x() + ((double)Height / (double)segment_line_num) * i * cos((angle_buf - 270) * M_PI / 180), RecPE.y() - ((double)Height / (double)segment_line_num) * i * sin((angle_buf - 270) * M_PI / 180));
			line = QLine(Ps, Pe);
			pf_min = this->mapToScene(QPointF(Ps.x(), Ps.y()));
			line_small_points.push_back(pf_min);
			pf_max = this->mapToScene(QPointF(Pe.x(), Pe.y()));
			line_big_points.push_back(pf_max);
		}
		painter->drawLine(line);
	}
	painter->setPen(QPen(QColor(0, 0, 255, 255), 3));
	painter->drawLine(P1, P2);	
	//绘制方向箭头
	float l = 22.0;//箭头长度
	double atn1 = atan2((P2.y() - P1.y()), (P2.x() - P1.x()));
	double atn2 = atan2((P2.x() - P1.x()), (P2.y() - P1.y()));
	QPointF Arrow1(P2.x() - l * cos(atn1 - 0.5), P2.y() - l * sin(atn1 - 0.5));
	QPointF Arrow2(P2.x() - l * sin(atn2 - 0.5), P2.y() - l * cos(atn2 - 0.5));
	painter->drawLine(P2, Arrow1);
	painter->drawLine(P2, Arrow2);
	painter->save();
	painter->translate(center);
	painter->setBrush(QBrush(fillColor));
	painter->rotate(-angle * 180 / M_PI);
	QPen tp = painter->pen();
	tp.setWidthF(0);
	tp.setColor(fillColor);
	painter->setPen(tp);
	thisPen.setWidthF(0);
	painter->drawRect(-Lenth / 2, -abs(Height / 2), Lenth, abs(Height));
	painter->restore();
}

void Caliper::GetCaliper(CaliperP& CP)
{			
	QPointF GetCenter = this->mapToScene(center.x(), center.y());
	CP.col = GetCenter.x();
	CP.row = GetCenter.y();
	CP.len1 = Lenth;
	CP.len2 = Height;
	CP.angle = angle;
	CP.x1 = P1.x();
	CP.y1 = P1.y();
	CP.x2 = P2.x();
	CP.y2 = P2.y();
	CP.height = Height;
	CP.pp1 = this->mapToScene(P1.x(), P1.y());
	CP.pp2 = this->mapToScene(P2.x(), P2.y());
}

void Caliper::SetCaliper(QPointF p1, QPointF p2)
{	
	P1 = p1;
	P2 = p2;
	ControlList[1] = new ControlItem(this, P1, 1);
	ControlList[2] = new ControlItem(this, P2, 2);
	UpDate(0);
	UpDate(1);
	UpDate(2);
	UpDate(3);	
}
#pragma endregion

