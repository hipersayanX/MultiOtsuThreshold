/* OtsuThreshold, Implementation of Multi Otsu Threshold in Qt/C++.
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
 *
 * This program is based in the original implementation
 * of Multi Otsu Threshold made by Yasunari Tosa for ImageJ:
 *
 *     http://imagej.net/Multi_Otsu_Threshold
*/

#include <iostream>
#include <cmath>
#include <QCoreApplication>
#include <QImage>
#include <QtDebug>

inline QVector<int> calculateHistogram(const QImage &image)
{
    QVector<int> histogram(256, 0);
    int size = image.width() * image.height();
    const quint8 *img = image.constBits();

    for (int i = 0; i < size; i++)
        histogram[img[i]]++;

    return histogram;
}

inline QVector<qreal> buildTables(const QVector<int> &histogram, const QSize &size)
{
    qreal P[histogram.size()][histogram.size()];
    qreal S[histogram.size()][histogram.size()];
    QVector<qreal> H(histogram.size() * histogram.size(), 0);

    // initialize
    for (int j = 0; j < histogram.size(); j++)
        for (int i = 0; i < histogram.size(); i++) {
            P[i][j] = 0;
            S[i][j] = 0;
        }

    // diagonal
    for (int i = 1; i < histogram.size(); i++) {
        P[i][i] = histogram[i];
        S[i][i] = i * histogram[i];
    }

    // calculate first row (row 0 is all zero)
    for (int i = 1; i < histogram.size() - 1; i++) {
        P[1][i + 1] = P[1][i] + histogram[i + 1];
        S[1][i + 1] = S[1][i] + (i + 1) * histogram[i + 1];
    }

    // using row 1 to calculate others
    for (int i = 2; i < histogram.size(); i++)
        for (int j = i + 1; j < histogram.size(); j++) {
            P[i][j] = P[1][j] - P[1][i - 1];
            S[i][j] = S[1][j] - S[1][i - 1];
        }

    int imageSize = size.width() * size.height();

    // now calculate H[i][j]
    for (int i = 1; i < histogram.size(); i++)
        for (int j = i + 1; j < histogram.size(); j++)
            if (P[i][j] != 0)
                H[j + i * histogram.size()] =
                        (S[i][j] * S[i][j])
                        / (P[i][j] * imageSize);

    return H;
}

inline QVector<int> otsu(const QImage &image, int nClasses)
{
    QVector<int> histogram = calculateHistogram(image);
    QVector<qreal> H = buildTables(histogram, image.size());
    const qreal *Hptr = H.constData();
    qreal maxSum = 0;
    QVector<int> otsu(nClasses - 1, 0);
    int limits[nClasses - 1];
    int index[nClasses - 1];

    for (int i = 0; i < otsu.size(); i++) {
        limits[i] = histogram.size() - nClasses + i;
        index[i] = i + 1;
    }

    while (index[0] < limits[0]) {
        qreal sum = 0;

        for (int i = 0; i < nClasses; i++) {
            int j = i < otsu.size()? index[i]: 255;
            int k = i > 0? index[i - 1]: 0;
            sum += Hptr[j + (k + 1) * histogram.size()];
        }

        if (maxSum < sum) {
            for (int i = 0; i < otsu.size(); i++)
                otsu[i] = index[i];

            maxSum = sum;
        }

        for (int i = otsu.size() - 1; i >= 0; i--) {
            index[i]++;

            if (index[i] < limits[i]) {
                for (int j = i + 1; j < otsu.size(); j++)
                    index[j] = index[j - 1] + 1;

                break;
            }
        }
    }

    return otsu;
}

QVector<int> threshold(const QImage &image,
                       const QVector<int> &otsu,
                       const QVector<int> &map)
{
    int size = image.width() * image.height();
    const quint8 *in = image.constBits();
    QVector<int> out(size);

    for (int i = 0; i < size; i++) {
        int value = -1;

        for (int j = 0; j < otsu.size(); j++)
            if (in[i] <= otsu[j]) {
                value = map[j];

                break;
            }

        out[i] = value < 0? map[otsu.size()]: value;
    }

    return out;
}

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);
    Q_UNUSED(a)

    QImage inImage("lena.png");
    inImage = inImage.convertToFormat(QImage::Format_Grayscale8);
    QImage outImage(inImage.size(), inImage.format());

    int nColors = 5;
    QVector<int> thresholds = otsu(inImage, nColors);
    qDebug() << thresholds;

    QVector<int> colors(nColors);

    for (int i = 0; i < nColors; i++)
        colors[i] = 255 * i / (nColors - 1);

    QVector<int> thresholded = threshold(inImage, thresholds, colors);

    quint8 *oImg = outImage.bits();
    int size = inImage.width() * inImage.height();

    for (int i = 0; i < size; i++)
        oImg[i] = thresholded[i];

    outImage.save("otsu.png");

    return EXIT_SUCCESS;
}
