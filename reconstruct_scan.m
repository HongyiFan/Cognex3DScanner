invK = inv(cameraParams.IntrinsicMatrix');

distanceBetweenFrames = 300;

objPts = [0 0 0];
k = 1;

listing = dir('LaserPoints*');
[~,idx] = sort([listing.datenum]);

nImg = size(listing, 1);

n = laserPlane(1:3);
d = laserPlane(4);

for i = 1:nImg
    pts = readPoints(listing(idx(i)).name);
    pts = undistortPoints(pts, cameraParams);

    for j = 1:size(pts, 1)
        u = [pts(j, 2), pts(j, 1), 1]';

        v = invK*u;
        lambda = d/dot(v, n);
        objPts(k, :) = lambda*v' + i*distanceBetweenFrames*stageCalib;
       
        k = k+1;
    end

    i
end

dlmwrite('scan.xyz', objPts, ' ');