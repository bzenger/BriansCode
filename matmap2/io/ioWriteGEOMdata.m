function ioWriteGEOMdata(varargin)

% FUNCTION ioWriteGEOMdata(filename,geomdata,['noprompt'])
%
% DESCRIPTION
% This function writes a .geom file
%
% INPUT
% filename       the filename(s) for saving the data (append extension .geom)
% geomdata       indices to the GEOM structure or direct structured data
% 
% OPTIONS
% noprompt       do not prompt before overwriting a file
%
% OUTPUT
% -
%
% NOTE
% Unlike ioWriteTS/ioWriteTSdata this function does not generate filenames
% automatically, hence you need to supply a filename. The function does not 
% have a overwrite protection neither, except that it prompts for permission 
% whenever overwriting an old geom file.
%
% GEOM STRUCTURE
%  .filename      - filename
%  .name          - the name of the data (where it came from)
%  .pts           - pts file
%  .cpts          - the conductivity tensors per point
%  .fac           - fac file
%  .cfac          - the conductivity tensors per element
%  .channels      - the channels file
%
% SEE ALSO ioReadGEOM, ioReadGEOMdata

% process the input

geomfiles = {};
geomdata = {};
noprompt = 0;

for p=1:nargin,
 
   fparam = varargin{p};

   if ischar(fparam)
       switch fparam
       case 'noprompt'
          noprompt = 1;
       otherwise
          geomfiles{end+1} = fparam;
       end
   end

   if isstruct(fparam), 
      geomdata{end+1} = fparam;
   end

   if isnumeric(fparam),
       global GEOM;
       for q=1:length(fparam),
            geomdata{end+1} = GEOM{fparam(q)};
	end
   end

   if iscell(fparam),
      for q=1:length(fparam),
          if isstruct(fparam{q}),
              geomdata{end+1} = fparam{q};
          end
          if ischar(fparam{q}),
              geomfiles{end+1} = fparam{q};
          end
      end 
   end
end

for p=1:length(geomfiles)
     [pname,fname,ext] = fileparts(geomfiles{p});
     switch ext,
     case '.fac'
         msgError('Use ioWriteFac to write .fac files',5);
         return;
     case '.pts'
         msgError('Use ioWritePts to write .pts files',5);
         return;
     case '.channels'
         msgError('Use ioWriteChannels to write .channels files',5);
         return;
     case '.geom'
         % do nothing
     otherwise
         geomfiles{p} = fullfile(pname,[fname '.geom']);
     end
end

for p=1:length(geomdata)
    if isfield(geomdata{p},'channels'),
        if ~isempty(geomdata{p}.channels),
            if isfield(geomdata{p},'cpts'),
                geomdata{p}.cpts = [];
                if isempty(geomdata{p}.cpts)
                    geomdata{p}.cpts(1).dim = 1;
                    geomdata{p}.cpts(1).type = 1;
                    geomdata{p}.cpts(1).cdata = geomdata{p}.channels;
                else
                    msgError('Cannot store channels AND conductivity data in one file',5);
                    return;
                end
            else
                geomdata{p}.cpts(1).dim = 1;
                geomdata{p}.cpts(1).type = 1;
                geomdata{p}.cpts(1).cdata = geomdata{p}.channels;
            end
        end
    end
end

if (length(geomfiles) == 1) & (length(geomdata) > 1),
     
    if exist(geomfiles{1},'file')    
        if noprompt ~= 1,			% be careful altering these statements as they allow for overwriting files
            question = sprintf('Do you want to replace: %s ?',geomfiles{1});
            result = utilQuestionYN(question);
        else
            result = 1;
        end       
        if (result==1),
            delete(geomfiles{1});			% deletion is done here as the mex functions cannot replace files
            mexWriteGEOM(geomfiles{1},geomdata);	
            fprintf(1,'Saving file: %s\n',geomfiles{1});
        else
            fprintf(1,'Could not store file, skipping this one\n');
        end
    else	
        mexWriteGEOM(geomfiles{1},geomdata);	
        fprintf(1,'Saving file: %s\n',geomfiles{1});
    end

elseif (length(geomfiles)==length(geomdata)),

   for p =1:length(geomfiles),
       if exist(geomfiles{p},'file')    
            if noprompt ~= 1,			% be careful altering these statements as they allow for overwriting files
                question = sprintf('Do you want to replace: %s ?',geomfiles{p});
                result = utilQuestionYN(question);
            else
                result = 1;
            end       
            if (result==1),
                delete(geomfiles{p});			% deletion is done here as the mex functions cannot replace files
                mexWriteGEOM(geomfiles{p},geomdata{p});	
                fprintf(1,'Saving file: %s\n',geomfiles{p});
            else
                fprintf(1,'Could not store file, skipping this one\n');
            end
        else	
            mexWriteGEOM(geomfiles{p},geomdata{p});	
            fprintf(1,'Saving file: %s\n',geomfiles{p});
        end
    end

else
    msgError('The number of geometries should match the number of filenames',5);
    return;
end