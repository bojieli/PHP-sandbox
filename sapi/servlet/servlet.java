/*
   +----------------------------------------------------------------------+
   | PHP version 4.0                                                      |
   +----------------------------------------------------------------------+
   | Copyright (c) 1997, 1998, 1999, 2000 The PHP Group                   |
   +----------------------------------------------------------------------+
   | This source file is subject to version 2.0 of the PHP license,       |
   | that is bundled with this package in the file LICENSE, and is        |
   | available at through the world-wide-web at                           |
   | http://www.php.net/license/2_0.txt.                                  |
   | If you did not receive a copy of the PHP license and are unable to   |
   | obtain it through the world-wide-web, please send a note to          |
   | license@php.net so we can mail you a copy immediately.               |
   +----------------------------------------------------------------------+
   | Author: Sam Ruby (rubys@us.ibm.com)                                  |
   +----------------------------------------------------------------------+
*/

package net.php;

import java.io.IOException;
import java.net.URLEncoder;
import java.util.Enumeration;
import javax.servlet.*;
import javax.servlet.http.*;

import java.lang.reflect.Method;

public class servlet extends HttpServlet {

    char slash=System.getProperty("file.separator").charAt(0);
    HttpServletRequest request;
    HttpServletResponse response;
    ServletInputStream stream;

    static int startup_count = 0;

    protected boolean display_source_mode = false;
    private Method addHeader;

    /******************************************************************/
    /*                          native methods                        */ 
    /******************************************************************/

    static { reflect.loadLibrary("servlet"); }
    native void startup();
    native long define(String name);
    native void send(String requestMethod, String queryString,
      String pathInfo, String pathTranslated,
      String contentType, int contentLength, String authUser,
      boolean display_source_mode);
    native void shutdown();

    /******************************************************************/
    /*                         sapi callbacks                         */ 
    /******************************************************************/

    String readPost(int bytes) {
      String result;
      if (!request.getMethod().equals("POST")) {
        result = request.getQueryString();
      } else { 
        Enumeration e = request.getParameterNames();
        result="";
        String concat="";
        while (e.hasMoreElements()) {
          String name = (String)e.nextElement();
          String value = request.getParameter(name);
          result+=concat+name+"="+URLEncoder.encode(value);
          concat="&";
        }
      }
      if (result == null) return "";
      return result; 
    }

    String readCookies() {
       reflect.setResult(define("request"), request);
       reflect.setResult(define("response"), response);
       return request.getHeader("cookie");
    }

    void header(String data) {

      // try to send the header using the most specific servlet API
      // as possible (some servlet engines will add a content type
      // header unless the setContentType method is called).
      try {
        if (data.startsWith("Content-type: ")) {
          response.setContentType(data.substring(data.indexOf(" ")+1));
        } else if (data.startsWith("Location: ")) {
          response.sendRedirect(data.substring(data.indexOf(" ")+1));
        } else {
          int colon = data.indexOf(": ");
          if (colon > 0) {
            try {
              addHeader.invoke(response, new Object[]
                { data.substring(0,colon), data.substring(colon+2) } );
            } catch (Exception e) {
              e.printStackTrace(System.err);
            }
          } else {
            response.getWriter().println(data);
          }
        }
      } catch (IOException e) {
        e.printStackTrace(System.err);
      }

    }

    void write(String data) {
      try {
        response.getWriter().print(data);
      } catch (IOException e) {
        e.printStackTrace(System.err);
      }
    }

    /******************************************************************/
    /*                        servlet interface                       */ 
    /******************************************************************/

    public void init(ServletConfig config) throws ServletException {
      super.init(config);
      if (0 == startup_count++) startup();

      // try to find the addHeader method (added in the servlet API 2.2)
      // otherwise settle for the setHeader method
      try {
        Class c = Class.forName("javax.servlet.http.HttpServletResponse");
        Method method[] = c.getDeclaredMethods();
        for (int i=0; i<method.length; i++) {
          if (method[i].getName().equals("addHeader")) {
            addHeader = method[i];
            break;
          }
          if (method[i].getName().equals("setHeader")) {
            addHeader = method[i];
          }
        }
      } catch (Exception e) {
        e.printStackTrace(System.err);
      }
    }

    public void service(HttpServletRequest request,
                        HttpServletResponse response) 
       throws ServletException
    {
       this.request=request;
       this.response=response;

       String servletPath=request.getServletPath();
       String contextPath=getServletContext().getRealPath(servletPath);

       send(request.getMethod(), request.getQueryString(),
            request.getPathInfo(), contextPath,
            request.getContentType(), request.getContentLength(),
	    request.getRemoteUser(), display_source_mode);

       try {
         if (stream != null) stream.close();
       } catch (IOException e) {
         throw new ServletException(e.toString());
       }
    }

    public void destroy() {
      if (0 == --startup_count) shutdown();
      super.destroy();
    }

}
