distanceBetweenFrames = 6000-3000;%steps or whatever unit used to control stage
squareSize = 25;

img1 = imread('NoLaser26.jpg');
img2 = imread('NoLaser27.jpg');

[img1, ~] = undistortImage(img1, cameraParams);
[img2, ~] = undistortImage(img2, cameraParams);

[imagePoints1, boardSize] = detectCheckerboardPoints(img1);
[imagePoints2, ~] = detectCheckerboardPoints(img2);

[worldPoints] = generateCheckerboardPoints(boardSize, squareSize); 

%imagePoints1 = [imagePoints1(:, 2), imagePoints1(:, 1)];
%imagePoints2 = [imagePoints2(:, 2), imagePoints2(:, 1)];

[R1, T1] = extrinsics(imagePoints1, worldPoints, cameraParams);
[R2, T2] = extrinsics(imagePoints2, worldPoints, cameraParams);

R1 = R1';
T1 = -T1;

R2 = R2';
T2 = -T2;

stageCalib = (T2 - T1)/distanceBetweenFrames;

imPoints = worldToImage(cameraParams, eye(3), [0 0 0], [T1; T2]);
img1 = insertMarker(img1, imPoints);
figure; imshow(img1);