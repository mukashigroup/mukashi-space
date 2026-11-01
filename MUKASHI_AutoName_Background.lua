-- Фоновый автонейминг - вешается в Background / Startup
-- Не показывает окно, просто каждые 1.5 сек обновляет имена в HUB

local function sync()
  local tmp = os.getenv("TEMP") or os.getenv("TMP") or "/tmp"
  local path = tmp .. "/mukashi_space_v3.json"
  local num = reaper.CountTracks(0)
  if num==0 then reaper.defer(sync) return end
  
  -- читаем текущий файл
  local f = io.open(path, "r")
  local current = {}
  if f then
    local txt = f:read("*all"); f:close()
    -- вытаскиваем позиции X/Y чтобы не затереть
    for id, x, y in txt:gmatch('"id":"(track_%d+)".-"x":([%d%.%-]+).-"y":([%d%.%-]+)') do
      current[id] = {x=tonumber(x), y=tonumber(y)}
    end
  end
  
  local json = '{"slots":['
  for i=0, num-1 do
    local track = reaper.GetTrack(0, i)
    local _, name = reaper.GetTrackName(track, "")
    if name=="" then name="Track "..(i+1) end
    name = name:gsub('"', "'"):gsub('\n',' ')
    local pos = current["track_"..i] or {x=(math.random()*60-30), y=i*5}
    if i>0 then json=json..',' end
    json=json..string.format('{"id":"track_%d","name":"%s","x":%.2f,"y":%.2f,"room":40,"wet":20,"color":%d,"active":true,"lastUpdate":%d}', i, name, pos.x, pos.y, 0xFF000000 + 0xAA5500 + i*800, reaper.time_precise()*1000)
  end
  for i=num,15 do
    json=json..','..string.format('{"id":"track_%d","name":"---","x":0,"y":0,"room":40,"wet":20,"color":%d,"active":false,"lastUpdate":0}', i, 0xFF222222)
  end
  json=json..']}'
  
  local out = io.open(path, "w")
  if out then out:write(json); out:close() end
  reaper.defer(sync)
end

reaper.defer(sync)
