-- new tests 2014-06-11

path = 'scripts'
if config.source_dir ~= nil then
	path = config.source_dir .. '\\' .. path
end
package.path = package.path .. ';' .. path .. '\\?.lua;' .. path .. '\\?\\init.lua'

-- require 'eressea.tests'
if config.rules ~= nil then
	require ('eressea.' .. config.rules .. '.tests')
end
require 'lunit'

lunit.main()
