<?xml version="1.0" encoding="UTF-8"?>

<xsl:stylesheet version="1.0" xmlns:xsl="http://www.w3.org/1999/XSL/Transform">
  <!-- Transformation templates -->

  <xsl:template match="string | int | float | bool">
    <xsl:attribute name="type">
      <xsl:value-of select="local-name()" />
    </xsl:attribute>

    <xsl:value-of select="@value" />
  </xsl:template>

  <xsl:template match="list">
    <xsl:attribute name="type">list</xsl:attribute>

    <xsl:for-each select="*">
      <elem>
        <xsl:apply-templates select="." />
      </elem>
    </xsl:for-each>
  </xsl:template>

  <xsl:template name="convert_attrs_verbose">
    <xsl:for-each select="attr">
      <property name="{@name}">
        <xsl:apply-templates select="*" />
      </property>
    </xsl:for-each>
  </xsl:template>

  <xsl:template match="attrs">
    <xsl:attribute name="type">attrs</xsl:attribute>
    <xsl:call-template name="convert_attrs_verbose" />
  </xsl:template>

  <!-- Transformation procedure -->

  <xsl:template match="/expr/attrs">
    <manifest version="2">
      <profiles>
        <xsl:for-each select="attr[@name='profiles']/attrs/attr">
          <profile name="{@name}"><xsl:value-of select="*/@value" /></profile>
        </xsl:for-each>
      </profiles>

      <services>
        <xsl:for-each select="attr[@name='services']/attrs/attr">
          <service name="{@name}">
            <name><xsl:value-of select="attrs/attr[@name='name']/*/@value" /></name>
            <pkg><xsl:value-of select="attrs/attr[@name='pkg']/*/@value" /></pkg>
            <type><xsl:value-of select="attrs/attr[@name='type']/*/@value" /></type>
            <dependsOn>
              <xsl:for-each select="attrs/attr[@name='dependsOn']/list/attrs">
                <mapping>
                  <xsl:for-each select="attr">
                    <xsl:element name="{@name}"><xsl:value-of select="*/@value" /></xsl:element>
                  </xsl:for-each>
                </mapping>
              </xsl:for-each>
            </dependsOn>
            <connectsTo>
              <xsl:for-each select="attrs/attr[@name='connectsTo']/list/attrs">
                <mapping>
                  <xsl:for-each select="attr">
                    <xsl:element name="{@name}"><xsl:value-of select="*/@value" /></xsl:element>
                  </xsl:for-each>
                </mapping>
              </xsl:for-each>
            </connectsTo>
            <providesContainers>
              <xsl:for-each select="attrs/attr[@name='providesContainers']/attrs/attr">
                <container name="{@name}">
                  <xsl:apply-templates select="*" />
                </container>
              </xsl:for-each>
            </providesContainers>
          </service>
        </xsl:for-each>
      </services>

      <infrastructure>
        <xsl:for-each select="attr[@name='infrastructure']/attrs/attr">
          <target name="{@name}">
            <properties>
              <xsl:apply-templates select="attrs/attr[@name='properties']/*" />
            </properties>
            <containers>
              <xsl:for-each select="attrs/attr[@name='containers']/attrs/attr">
                <container name="{@name}">
                  <xsl:apply-templates select="*" />
                </container>
              </xsl:for-each>
            </containers>

            <system><xsl:value-of select="attrs/attr[@name='system']/*/@value" /></system>
            <numOfCores><xsl:value-of select="attrs/attr[@name='numOfCores']/*/@value" /></numOfCores>
            <clientInterface><xsl:value-of select="attrs/attr[@name='clientInterface']/*/@value" /></clientInterface>
            <targetProperty><xsl:value-of select="attrs/attr[@name='targetProperty']/*/@value" /></targetProperty>
          </target>
        </xsl:for-each>
      </infrastructure>

      <serviceMappings>
        <xsl:for-each select="attr[@name='serviceMappings']/list/attrs">
          <mapping>
            <xsl:for-each select="attr">
              <xsl:element name="{@name}"><xsl:value-of select="*/@value" /></xsl:element>
            </xsl:for-each>
          </mapping>
        </xsl:for-each>
      </serviceMappings>

      <snapshotMappings>
        <xsl:for-each select="attr[@name='snapshotMappings']/list/attrs">
          <mapping>
            <xsl:for-each select="attr">
              <xsl:element name="{@name}"><xsl:value-of select="*/@value" /></xsl:element>
            </xsl:for-each>
          </mapping>
        </xsl:for-each>
      </snapshotMappings>
    </manifest>
  </xsl:template>
</xsl:stylesheet>
