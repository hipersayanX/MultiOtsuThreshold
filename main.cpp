/* OtsuThreshold, Implementation of Multi level Otsu Threshold in Qt/C++.
 * Copyright (C) 2015  Gonzalo Exequiel Pedone
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 *
 * Email   : hipersayan DOT x AT gmail DOT com
 * Web-Site: http://github.com/hipersayanX/MultiOtsuThreshold
*/

#include <algorithm>
#include <QCoreApplication>
#include <QElapsedTimer>
#include <QImage>
#include <QtMath>
#include <QtDebug>

inline QVector<int> histogram(const QImage &image)
{
    QVector<int> histogram(256, 0);

    for (int y = 0; y < image.height(); y++) {
        const quint8 *line = reinterpret_cast<const quint8 *>(image.constScanLine(y));

        for (int x = 0; x < image.width(); x++)
            histogram[line[x]]++;
    }

    return histogram;
}

inline void printHistogram(int width, int height,
                           const QVector<int> &histogram,
                           const QVector<int> &thresholds=QVector<int>())
{
    // Create the graph.
    QString graph((width + 1) * height, ' ');

    // Split each line.
    for (int y = 0; y < height; y++)
        graph[width + y * (width + 1)] = '\n';

    int maxValue = *std::max_element(histogram.constBegin(), histogram.constEnd());

    // Draw values.
    for (int x = 0; x < width; x++) {
        int h = (height - 1)
                * histogram[(histogram.size() - 1) * x / (width - 1)]
                / maxValue;

        for (int y = height - 1; y >= (height - h - 1); y--)
            graph[x + y * (width + 1)] = '*';
    }

    // Draw the trhesholds.
    foreach (int x, thresholds) {
        int w = (width - 1) * x / (histogram.size() - 1);

        for (int y = 0; y < height; y++)
            graph[w + y * (width + 1)] = '|';
    }

    // Print the graph
    qDebug() << graph.toStdString().c_str();
}

inline QVector<qreal> buildTables(const QVector<int> &histogram)
{
    // Create cumulative sum tables.
    QVector<quint64> P(histogram.size() + 1);
    QVector<quint64> S(histogram.size() + 1);

    quint64 sumP = 0;
    quint64 sumS = 0;

    for (int i = 0; i < histogram.size(); i++) {
        sumP += quint64(histogram[i]);
        sumS += quint64(i * histogram[i]);
        P[i + 1] = sumP;
        S[i + 1] = sumS;
    }

    // Calculate the between-class variance for the interval u-v
    QVector<qreal> H(histogram.size() * histogram.size(), 0.);

    for (int u = 0; u < histogram.size(); u++) {
        qreal *hLine = H.data() + u * histogram.size();

        for (int v = u + 1; v < histogram.size(); v++)
            if (P[v] == P[u])
                hLine[v] = S[v] == S[u]? qQNaN(): qInf();
            else
                hLine[v] = qPow(S[v] - S[u], 2) / (P[v] - P[u]);
    }

    return H;
}

void for_loop(qreal *maxSum,
              QVector<int> *thresholds,
              const QVector<qreal> &H,
              int u,
              int vmax,
              int level,
              int levels,
              QVector<int> *index)
{
    int classes = index->size() - 1;

    for (int i = u; i < vmax; i++) {
        (*index)[level] = i;

        if (level + 1 >= classes) {
            // Reached the end of the for loop.

            // Calculate the quadratic sum of al intervals.
            qreal sum = 0;

            for (int c = 0; c < classes; c++) {
                int u = index->at(c);
                int v = index->at(c + 1);
                sum += H[v + u * levels];
            }

            if (*maxSum < sum) {
                // Return calculated threshold.
                *thresholds = index->mid(1, thresholds->size());
                *maxSum = sum;
            }
        } else
            // Start a new for loop level, one position after current one.
            for_loop(maxSum, thresholds, H, i + 1, vmax + 1, level + 1, levels, index);
    }
}

inline QVector<int> otsu(QVector<int> histogram,
                         int classes)
{
    qreal maxSum = 0;
    QVector<int> thresholds(classes - 1, 0);
    QVector<qreal> H = buildTables(histogram);
    QVector<int> index(classes + 1);
    index[index.size() - 1] = histogram.size() - 1;
    for_loop(&maxSum, &thresholds, H, 0, histogram.size() - classes, 0, histogram.size(), &index);

    return thresholds;
}


inline QImage threshold(const QImage &src,
                        const QVector<int> &thresholds,
                        const QVector<int> &colors)
{
    QImage dst(src.size(), src.format());

    for (int y = 0; y < src.height(); y++) {
        const quint8 *srcLine = src.constScanLine(y);
        quint8 *dstLine = dst.scanLine(y);

        for (int x = 0; x < src.width(); x++) {
            int value = -1;

            for (int j = 0; j < thresholds.size(); j++)
                if (srcLine[x] <= thresholds[j]) {
                    value = colors[j];

                    break;
                }

            dstLine[x] = quint8(value < 0? colors[thresholds.size()]: value);
        }
    }

    return dst;
}

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);
    Q_UNUSED(a)

    QImage inImage(":/otsu/lena.png");
    inImage = inImage.convertToFormat(QImage::Format_Grayscale8);

    QElapsedTimer total;
    total.start();

    QVector<int> hist = histogram(inImage);
    int classes = 4;
    QVector<int> thresholds = otsu(hist, classes);
    QVector<int> colors(classes);

    for (int i = 0; i < classes; i++)
        colors[i] = 255 * i / (classes - 1);

    QImage thresholded = threshold(inImage, thresholds, colors);
    qint64 t = total.elapsed();

    thresholded.save("otsu.png");

    printHistogram(128, 12, hist, thresholds);
    qDebug() << "Thresholds:" << thresholds;
    qDebug() << "Time elapsed:" << t << "ms";

    return EXIT_SUCCESS;
}
