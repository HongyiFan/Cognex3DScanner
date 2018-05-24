function out = readPoints(fname)

    fid = fopen(fname, 'rt');
    datacell = textscan(fid, '%f%f', 'TreatAsEmpty', '#ERR');
    fclose(fid);
    
    out = [datacell{1}, datacell{2}];
    out(any(isnan(out), 2), :) = [];%remove nan entries

end

