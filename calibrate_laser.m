close all;
squareSize = 25;
planeTolerance = 25;

invK = inv(cameraParams.IntrinsicMatrix');

listing = dir('./laserCalibration/NoLaser*');

nImg = size(listing, 1);
fnames = extractfield(listing, 'name');

txtFiles = dir('./laserCalibration/LaserPoints*');
laserPoints = [0 0 0];
k = 1;

for i = 1:nImg
    img = imread(fnames{i});
    [img, ~] = undistortImage(img, cameraParams);
    [imagePoints, boardSize] = detectCheckerboardPoints(img);
    [worldPoints] = generateCheckerboardPoints(boardSize, squareSize); 
    [R, T] = extrinsics(imagePoints, worldPoints, cameraParams);
    
    
    pts = readPoints(txtFiles(i).name);
    pts = undistortPoints(pts, cameraParams);
    
    mask = zeros(size(img));
    mask = mask(:, :, 1);
    idx = sub2ind(size(mask), round(imagePoints(:, 2)), round(imagePoints(:, 1)));
    mask(idx) = 1;
    mask = bwconvhull(mask);
    
    R = R';
    T = -T;
    n = R*[0 0 1]';
    d = -dot(T, n);
    
    firstPoint = k;
    
    for j = 1:size(pts, 1)
        if mask(round(pts(j, 1)), round(pts(j, 2))) > 0
            pt = [pts(j, 2), pts(j, 1), 1]';
            v = invK*pt;
            
            lambda = -d/dot(v, n);
          
            laserPoints(k, :) = lambda*v';
            k = k+1;
        end
    end
    
    %back project
%     imPoints = worldToImage(cameraParams, eye(3), [0 0 0], laserPoints(firstPoint:end, :));
%     imPoints = [imPoints(:, 2), imPoints(:, 1)];
%     img = insertMarker(img, imPoints);
%     figure; imshow(img);
end

dlmwrite('triangulated.xyz', laserPoints, ' ');

%fit a plane to the detected laserPoints
model = pcfitplane(pointCloud(laserPoints), planeTolerance);
laserPlane = model.Parameters;