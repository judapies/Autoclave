<?xml version='1.0' encoding='UTF-8'?>
<Project Type="Project" LVVersion="11008008">
	<Item Name="My Computer" Type="My Computer">
		<Property Name="server.app.propertiesEnabled" Type="Bool">true</Property>
		<Property Name="server.control.propertiesEnabled" Type="Bool">true</Property>
		<Property Name="server.tcp.enabled" Type="Bool">false</Property>
		<Property Name="server.tcp.port" Type="Int">0</Property>
		<Property Name="server.tcp.serviceName" Type="Str">My Computer/VI Server</Property>
		<Property Name="server.tcp.serviceName.default" Type="Str">My Computer/VI Server</Property>
		<Property Name="server.vi.callsEnabled" Type="Bool">true</Property>
		<Property Name="server.vi.propertiesEnabled" Type="Bool">true</Property>
		<Property Name="specify.custom.address" Type="Bool">false</Property>
		<Item Name="Autoclave.ico" Type="Document" URL="/C/Users/maria consuelo/Downloads/Autoclave.ico"/>
		<Item Name="Control_Autoclave.vi" Type="VI" URL="../Control_Autoclave.vi"/>
		<Item Name="Dependencies" Type="Dependencies">
			<Item Name="BulkUSB_MN (subvi).vi" Type="VI" URL="../BulkUSB_MN (subvi).vi"/>
			<Item Name="mpusbapi.dll" Type="Document" URL="/C/pic32_solutions/USB Tools/MCHPUSB Custom Driver/Mpusbapi/Dll/Borland_C/mpusbapi.dll"/>
		</Item>
		<Item Name="Build Specifications" Type="Build">
			<Item Name="Control_Autoclave" Type="EXE">
				<Property Name="App_copyErrors" Type="Bool">true</Property>
				<Property Name="App_INI_aliasGUID" Type="Str">{1B525D00-30A6-4A12-8F25-470A2F53D83B}</Property>
				<Property Name="App_INI_GUID" Type="Str">{E0F55E20-1CDF-4C63-8795-E0A971894E54}</Property>
				<Property Name="Bld_buildCacheID" Type="Str">{9F6E9CF4-3964-49D6-A8F8-4E1662AD3CF1}</Property>
				<Property Name="Bld_buildSpecDescription" Type="Str">Programa para realizar configuraciones y control de autoclave marca JP Inglobal.</Property>
				<Property Name="Bld_buildSpecName" Type="Str">Control_Autoclave</Property>
				<Property Name="Bld_excludeLibraryItems" Type="Bool">true</Property>
				<Property Name="Bld_excludePolymorphicVIs" Type="Bool">true</Property>
				<Property Name="Bld_localDestDir" Type="Path">../builds/NI_AB_PROJECTNAME/Control_Autoclave</Property>
				<Property Name="Bld_localDestDirType" Type="Str">relativeToCommon</Property>
				<Property Name="Bld_modifyLibraryFile" Type="Bool">true</Property>
				<Property Name="Bld_previewCacheID" Type="Str">{90EC97EB-E0CC-42D4-8444-84D7067C301E}</Property>
				<Property Name="Destination[0].destName" Type="Str">JP Autoclave.exe</Property>
				<Property Name="Destination[0].path" Type="Path">../builds/NI_AB_PROJECTNAME/Control_Autoclave/JP Autoclave.exe</Property>
				<Property Name="Destination[0].preserveHierarchy" Type="Bool">true</Property>
				<Property Name="Destination[0].type" Type="Str">App</Property>
				<Property Name="Destination[1].destName" Type="Str">Support Directory</Property>
				<Property Name="Destination[1].path" Type="Path">../builds/NI_AB_PROJECTNAME/Control_Autoclave/data</Property>
				<Property Name="DestinationCount" Type="Int">2</Property>
				<Property Name="Exe_iconItemID" Type="Ref">/My Computer/Autoclave.ico</Property>
				<Property Name="Source[0].itemID" Type="Str">{96863ECF-2A1F-457B-8246-C0869728A6B3}</Property>
				<Property Name="Source[0].type" Type="Str">Container</Property>
				<Property Name="Source[1].destinationIndex" Type="Int">0</Property>
				<Property Name="Source[1].itemID" Type="Ref">/My Computer/Control_Autoclave.vi</Property>
				<Property Name="Source[1].sourceInclusion" Type="Str">TopLevel</Property>
				<Property Name="Source[1].type" Type="Str">VI</Property>
				<Property Name="SourceCount" Type="Int">2</Property>
				<Property Name="TgtF_companyName" Type="Str">JP Bioingenieria SAS</Property>
				<Property Name="TgtF_fileDescription" Type="Str">Control_Autoclave</Property>
				<Property Name="TgtF_fileVersion.major" Type="Int">1</Property>
				<Property Name="TgtF_internalName" Type="Str">Control_Autoclave</Property>
				<Property Name="TgtF_legalCopyright" Type="Str">Copyright © 2015 JP Bioingenieria SAS</Property>
				<Property Name="TgtF_productName" Type="Str">Control_Autoclave</Property>
				<Property Name="TgtF_targetfileGUID" Type="Str">{74AFD6B2-96DD-44C5-AA36-679BEB0558D1}</Property>
				<Property Name="TgtF_targetfileName" Type="Str">JP Autoclave.exe</Property>
			</Item>
		</Item>
	</Item>
</Project>
