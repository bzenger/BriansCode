function result = ioWritePts(filename,Pts)

% function result = ioWriteFac(filename,Pts)
%
% this function writes a fac-geometry file to file
%
% INPUT
%  filename - name of the output file
%  Pts      - matrix containing the node position data
%
% OUTPUT
%  result   - is one in case of success or zero in case of failure


% Test whether Pts needs to be transposed

if size(Pts,1) ~= 3,
    Pts = Pts';
    if size(Pts,1) ~= 3,
	msgError('Pts does not have proper dimensions',4);
        result = 0;
        return
    end
end

% First ensure that the file has the correct extension

[pn,fn,ext] = fileparts(filename);

% Try to correct a faulty file name
if strcmp(ext,'pts') == 0, % no match
    filename = fullfile(pn,[fn '.pts']);
end

FID = fopen(filename,'w');

if FID < 1,
    err = sprintf('Could not open file : %s\n',filename);
    msgError(err,3);
    result = 0;
    return
end

fprintf(FID,'% 7.2f % 7.2f % 7.2f\n',Pts);
fclose(FID);

result = 1; % everything went OK
return
